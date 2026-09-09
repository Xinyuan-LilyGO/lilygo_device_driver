/*
 * @Description: T-Display-P4 V2 板级设备驱动实现
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:51:14
 * @LastEditTime: 2026-09-02 17:16:04
 * @License: GPL 3.0
 */
#include "device/t_display_p4/driver.h"

#include <cstdio>

#include "core/logger.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_master.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

namespace lilygo_device_driver {
namespace gpio = t_display_p4::gpio;
namespace device = t_display_p4::device;
namespace {

constexpr uint32_t kInitializationShutdownTimeoutMs = 5 * 1000;
constexpr uint8_t kLr2021ExpectedVersionMajor = 0x01;
constexpr uint8_t kLr2021ExpectedVersionMinor = 0x18;

}  // namespace

bool TDisplayP4Driver::Init(InitMode mode) {
  CreateDrivers();
  const int64_t start_time_us = platform_hal_->GetSystemTimeUs();
  const bool result = InitDrivers(mode);
  const int64_t elapsed_time_us =
      platform_hal_->GetSystemTimeUs() - start_time_us;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      "TDisplayP4Driver init (mode: %s, result: %s, elapsed: "
      "%lld ms)\n",
      mode == InitMode::kAsync ? "async" : "sync",
      result ? (mode == InitMode::kAsync ? "tasks scheduled" : "success")
             : "failed",
      static_cast<long long>(elapsed_time_us / 1000));
  return result;
}

bool TDisplayP4Driver::InitMinimal() {
  CreateDrivers();
  // 初始化失败保留公共电源，允许后续重试。
  return InitMinimalDrivers();
}

bool TDisplayP4Driver::InitAxp517() {
  CreateDrivers();
  if (IsAxp517Ready()) {
    return true;
  }
  if (!InitPower() || !chip_.axp517->Init()) {
    status_.axp517.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAxp517 failed\n");
    return false;
  }

  const cpp_bus_driver::Axp517::AdcChannel adc_channel = {
      .battery_discharge_current_measure = true,
      .battery_charge_current_measure = true,
      .chip_temperature_measure = true,
      .ts_value_measure = true,
      .battery_voltage_measure = true,
  };
  bool result = true;
  result &= chip_.axp517->SetAdcChannel(adc_channel);
  result &= chip_.axp517->SetBoostVoltage(5000);
  result &= chip_.axp517->SetForceRbfetEnable(false);
  result &= chip_.axp517->SetBoostEnable(false);
  result &= chip_.axp517->SetTypeCDetectEnable(true);
  result &= chip_.axp517->SetVbusDetectEnable(true);
  result &= chip_.axp517->SetPdRole(false, false);
  status_.axp517.init_flag = result;
  if (!result) {
    chip_.axp517->Deinit(false);
  }
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitAxp517 success\n" : "InitAxp517 failed\n");
  return result;
}

bool TDisplayP4Driver::InitXl9535() {
  CreateDrivers();
  if (IsXl9535Ready()) {
    return true;
  }
  if (!InitPower() || !chip_.xl9535->Init()) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  }

  struct OutputConfig {
    cpp_bus_driver::Xl95x5::Pin pin;
    uint8_t level;
  };
  // 先写输出锁存器，再切换方向，避免上电默认高电平误开启外设。
  constexpr OutputConfig kOutputs[] = {
      {gpio::xl9535::kGpsRst, 1},
      {gpio::xl9535::kGpsWakeUp, 0},
      {gpio::xl9535::kEsp32c5Boot, 1},
      {gpio::xl9535::kEsp32c5En, 0},
      {gpio::xl9535::kScreenRst, device::xl9535::kResetAsserted},
      {gpio::xl9535::kNs4150En, 0},
      {gpio::xl9535::kTouchRst, device::xl9535::kResetAsserted},
      {gpio::xl9535::kLed, 1},
      {gpio::xl9535::kUsbPhyPowerEn, 1},
      {gpio::xl9535::kLr2021Rst, device::xl9535::kResetAsserted},
      {gpio::xl9535::kLr2021PowerEn, 0},
      {gpio::xl9535::kSdPowerEn, 0},
  };
  bool result = true;
  for (const auto& output : kOutputs) {
    result &= chip_.xl9535->GpioWrite(output.pin, output.level);
  }
  if (result) {
    for (const auto& output : kOutputs) {
      result &= chip_.xl9535->SetGpioMode(
          output.pin, cpp_bus_driver::Xl95x5::Mode::kOutput);
    }
  }
  status_.xl9535.init_flag = result;
  if (!result) {
    chip_.xl9535->Deinit(false);
  }
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitXl9535 success\n" : "InitXl9535 failed\n");
  return result;
}

bool TDisplayP4Driver::InitIcm20948() {
  // 暂时不添加 BHI260AP、QMC6309 的驱动，后续硬件替换为 ICM20948 后再接入。
  // 当前仅预留接口，不将设备标记为就绪。
  return false;
}

bool TDisplayP4Driver::InitSy7200a() {
  if (chip_.sy7200a != nullptr && chip_.sy7200a->IsInitialized()) {
    status_.sy7200a.init_flag = true;
    return true;
  }
  if (chip_.sy7200a == nullptr) {
    status_.sy7200a.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSy7200a failed\n");
    return false;
  }

  cpp_bus_driver::Pwm::Config config;
  config.timer = LEDC_TIMER_0;
  config.channel = LEDC_CHANNEL_0;
  config.frequency_hz = device::sy7200a::kPwmFrequencyHz;
  if (!chip_.sy7200a->Init(config)) {
    status_.sy7200a.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSy7200a failed\n");
    return false;
  }

  status_.sy7200a.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSy7200a success\n");
  return true;
}

bool TDisplayP4Driver::InitEs8389() {
  if (IsEs8389Ready()) {
    return true;
  }
  if ((bus_.xl9535_i2c_bus == nullptr) || (bus_.es8389_i2s_bus == nullptr)) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  i2c_master_bus_handle_t i2c_bus_handle = bus_.xl9535_i2c_bus->bus_handle();
  if (i2c_bus_handle == nullptr) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  audio_codec_i2c_cfg_t i2c_cfg = {
      .port = static_cast<uint8_t>(I2C_NUM_1),
      .addr = static_cast<uint8_t>(device::es8389::kI2cAddress << 1),
      .bus_handle = i2c_bus_handle,
  };
  es8389_ctrl_if_ = audio_codec_new_i2c_ctrl(&i2c_cfg);

  if (es8389_ctrl_if_ == nullptr ||
      !bus_.es8389_i2s_bus->Init(
          [](int value) -> i2s_mclk_multiple_t {
            switch (value) {
              case 128:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_128;
              case 192:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_192;
              case 256:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_256;
              case 384:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_384;
              case 512:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_512;
              case 576:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_576;
              case 768:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_768;
              case 1024:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_1024;
              case 1152:
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_1152;
              default:
                LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
                    "Value out of range\n");
                return i2s_mclk_multiple_t::I2S_MCLK_MULTIPLE_256;
            }
          }(device::es8389::kMclkMultiple),
          device::es8389::kSampleRate,
          [](int value) -> i2s_data_bit_width_t {
            switch (value) {
              case 8:
                return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_8BIT;
              case 16:
                return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT;
              case 24:
                return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_24BIT;
              case 32:
                return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_32BIT;
              default:
                LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
                    "Value out of range\n");
                return i2s_data_bit_width_t::I2S_DATA_BIT_WIDTH_16BIT;
            }
          }(device::es8389::kBitsPerSample))) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  audio_codec_i2s_cfg_t i2s_cfg = {
      .port = static_cast<uint8_t>(bus_.es8389_i2s_bus->port()),
      .rx_handle = bus_.es8389_i2s_bus->rx_handle(),
      .tx_handle = bus_.es8389_i2s_bus->tx_handle(),
      .clk_src = static_cast<int>(I2S_CLK_SRC_DEFAULT),
  };
  es8389_data_if_ = audio_codec_new_i2s_data(&i2s_cfg);
  if (es8389_data_if_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    status_.es8389.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }
  bool amplifier_pin_initialized = true;
  amplifier_pin_initialized &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, 0);
  amplifier_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kNs4150En, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!amplifier_pin_initialized) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  es8389_gpio_if_ = audio_codec_new_gpio();
  if (es8389_gpio_if_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  es8389_codec_cfg_t codec_cfg = {
      .ctrl_if = es8389_ctrl_if_,
      .gpio_if = es8389_gpio_if_,
      .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
      .pa_pin = -1,
      .pa_reverted = false,
      .master_mode = false,
      .use_mclk = true,
      .digital_mic = false,
      .invert_mclk = false,
      .invert_sclk = false,
      .hw_gain =
          {
              .pa_voltage = 3.3f,
              .codec_dac_voltage = 3.3f,
              .pa_gain = 0.0f,
          },
      .no_dac_ref = false,
      .mclk_div = device::es8389::kMclkMultiple,
  };
  es8389_codec_if_ = es8389_codec_new(&codec_cfg);
  if (es8389_codec_if_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  esp_codec_dev_cfg_t output_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_OUT,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_output_codec_dev_ = esp_codec_dev_new(&output_dev_cfg);
  if (es8389_output_codec_dev_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  esp_codec_dev_cfg_t input_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_input_codec_dev_ = esp_codec_dev_new(&input_dev_cfg);
  if (es8389_input_codec_dev_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
    return false;
  }

  esp_codec_dev_sample_info_t output_sample_info = {
      .bits_per_sample = device::es8389::kBitsPerSample,
      .channel = device::es8389::kChannel,
      .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) |
                      ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1),
      .sample_rate = device::es8389::kSampleRate,
      .mclk_multiple = device::es8389::kMclkMultiple,
  };
  esp_codec_dev_sample_info_t input_sample_info = output_sample_info;
  const bool output_opened = esp_codec_dev_open(es8389_output_codec_dev_,
                                 &output_sample_info) == ESP_CODEC_DEV_OK;
  const bool input_opened =
      output_opened && esp_codec_dev_open(es8389_input_codec_dev_,
                           &input_sample_info) == ESP_CODEC_DEV_OK;
  bool result = output_opened && input_opened;
  if (result) {
    result &= (esp_codec_dev_set_out_vol(es8389_output_codec_dev_, 100) ==
               ESP_CODEC_DEV_OK);
    result &= (esp_codec_dev_set_in_gain(es8389_input_codec_dev_, 20.0f) ==
               ESP_CODEC_DEV_OK);
  }

  // 关闭操作属于回滚，必须尽量释放所有已经打开的通道。
  if (input_opened) {
    result &=
        (esp_codec_dev_close(es8389_input_codec_dev_) == ESP_CODEC_DEV_OK);
  }
  if (output_opened) {
    result &=
        (esp_codec_dev_close(es8389_output_codec_dev_) == ESP_CODEC_DEV_OK);
  }
  result &= SetNs4150Enabled(false);
  es8389_operating_mode_ = Es8389OperatingMode::kSleep;
  status_.es8389.init_flag = result;
  if (result) {
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitEs8389 success\n");
  } else {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8389 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitLr2021() {
  if (IsLr2021Ready()) {
    return true;
  }
  if (chip_.lr2021 == nullptr) {
    return false;
  }

  status_.lr2021.init_flag = false;
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    return false;
  }
  bool reset_pin_initialized = true;
  reset_pin_initialized &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kLr2021PowerEn, 1);
  platform_hal_->DelayMs(10);
  reset_pin_initialized &= platform_hal_->SetGpioMode(
      gpio::lr2021::kInt, cpp_bus_driver::PlatformHal::GpioMode::kInput);
  reset_pin_initialized &= chip_.xl9535->GpioWrite(
      gpio::xl9535::kLr2021Rst, device::xl9535::kResetAsserted);
  reset_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kLr2021Rst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!reset_pin_initialized ||
      !chip_.lr2021->Init(device::lr2021::kSpiFrequencyHz)) {
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kLr2021Rst, device::xl9535::kResetAsserted);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitLr2021 transport failed\n");
    return false;
  }

  constexpr uint8_t kDetectionAttempts = 10;
  lr20xx_system_version_t version = {};
  bool detected = false;
  for (uint8_t attempt = 0; attempt < kDetectionAttempts; ++attempt) {
    version = {};
    detected = chip_.lr2021->Invoke(lr20xx_system_get_version, &version) ==
                   LR20XX_STATUS_OK &&
               version.major == kLr2021ExpectedVersionMajor &&
               version.minor == kLr2021ExpectedVersionMinor;
    if (detected) {
      break;
    }
    if (attempt + 1U < kDetectionAttempts) {
      platform_hal_->DelayMs(10);
      if (!chip_.lr2021->Reset()) {
        break;
      }
    }
  }

  if (!detected) {
    chip_.lr2021->Deinit(false);
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kLr2021Rst, device::xl9535::kResetAsserted);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 chip detection failed (fw: %u.%u)\n",
        static_cast<unsigned>(version.major),
        static_cast<unsigned>(version.minor));
    return false;
  }

  const auto configure_rf_switch =
      [this](
          lr20xx_system_dio_t dio, lr20xx_system_dio_rf_switch_cfg_t config) {
        if (chip_.lr2021->Invoke(lr20xx_system_set_dio_function, dio,
                LR20XX_SYSTEM_DIO_FUNC_RF_SWITCH,
                LR20XX_SYSTEM_DIO_DRIVE_NONE) != LR20XX_STATUS_OK) {
          return false;
        }
        return chip_.lr2021->Invoke(lr20xx_system_set_dio_rf_switch_cfg, dio,
                   config) == LR20XX_STATUS_OK;
      };
  constexpr auto kCalibrationMask =
      static_cast<lr20xx_system_calibration_mask_t>(
          LR20XX_SYSTEM_CALIB_LF_RC_MASK | LR20XX_SYSTEM_CALIB_HF_RC_MASK |
          LR20XX_SYSTEM_CALIB_PLL_MASK | LR20XX_SYSTEM_CALIB_AAF_MASK |
          LR20XX_SYSTEM_CALIB_MU_MASK | LR20XX_SYSTEM_CALIB_PA_OFF_MASK);

  const lr20xx_system_sleep_cfg_t sleep_config = {
      .is_clk_32k_enabled = false,
      .is_ram_retention_enabled = true,
  };
  bool result = true;
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_standby_mode,
                 LR20XX_SYSTEM_STANDBY_MODE_RC) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_tcxo_mode,
                 LR20XX_SYSTEM_TCXO_CTRL_3_3V, 32768U) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_reg_mode,
                 LR20XX_SYSTEM_REG_MODE_DCDC) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_radio_common_set_rx_tx_fallback_mode,
                 LR20XX_RADIO_FALLBACK_STDBY_RC) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_clear_irq_status,
                 LR20XX_SYSTEM_IRQ_ALL_MASK) == LR20XX_STATUS_OK);
  result &= (chip_.lr2021->Invoke(lr20xx_system_calibrate, kCalibrationMask) ==
             LR20XX_STATUS_OK);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_6, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_HF);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_7, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_HF);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_8, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_LF |
                               LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_LF);
  result &= configure_rf_switch(
      LR20XX_SYSTEM_DIO_10, LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_RX_HF |
                                LR20XX_SYSTEM_DIO_RF_SWITCH_WHEN_TX_HF);
  result &= (chip_.lr2021->Invoke(lr20xx_system_set_dio_function,
                 LR20XX_SYSTEM_DIO_11, LR20XX_SYSTEM_DIO_FUNC_IRQ,
                 LR20XX_SYSTEM_DIO_DRIVE_NONE) == LR20XX_STATUS_OK);
  result &=
      (chip_.lr2021->Invoke(lr20xx_system_set_dio_irq_cfg, LR20XX_SYSTEM_DIO_11,
           LR20XX_SYSTEM_IRQ_NONE) == LR20XX_STATUS_OK);
  result &= chip_.lr2021->SetSleep(sleep_config);
  if (!result) {
    chip_.lr2021->Deinit(false);
    chip_.xl9535->GpioWrite(
        gpio::xl9535::kLr2021Rst, device::xl9535::kResetAsserted);
    chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 hardware configuration failed\n");
    return false;
  }

  status_.lr2021.init_flag = true;
  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "InitLr2021 success (fw: %u.%u)\n",
      static_cast<unsigned>(version.major),
      static_cast<unsigned>(version.minor));
  return true;
}

bool TDisplayP4Driver::InitL76k() {
  if (IsL76kReady()) {
    return true;
  }
  if (!status_.xl9535.init_flag || chip_.xl9535 == nullptr) {
    status_.l76k.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
    return false;
  }
  bool reset_initialized = chip_.xl9535->GpioWrite(gpio::xl9535::kGpsRst, 0);
  reset_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kGpsRst, cpp_bus_driver::Xl95x5::Mode::kOutput);
  platform_hal_->DelayMs(10);
  reset_initialized &= chip_.xl9535->GpioWrite(gpio::xl9535::kGpsRst, 1);
  if (!reset_initialized) {
    return false;
  }
  platform_hal_->DelayMs(100);
  bool wakeup_pin_initialized = true;
  wakeup_pin_initialized &=
      chip_.xl9535->GpioWrite(gpio::xl9535::kGpsWakeUp, 0);
  wakeup_pin_initialized &= chip_.xl9535->SetGpioMode(
      gpio::xl9535::kGpsWakeUp, cpp_bus_driver::Xl95x5::Mode::kOutput);
  if (!wakeup_pin_initialized) {
    status_.l76k.init_flag = false;
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
    return false;
  }
  if (!chip_.l76k->Init()) {
    if (!bus_.l76k_uart_bus->SetBaudRate(115200)) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "SetBaudRate failed\n");
      return false;
    }
    if (!chip_.l76k->Init()) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
      return false;
    } else {
      bool result = true;
      result &=
          chip_.l76k->SetBaudRate(cpp_bus_driver::L76k::BaudRate::kBr115200Bps);
      result &= chip_.l76k->SetUpdateFrequency(
          cpp_bus_driver::L76k::UpdateFreq::kFreq5Hz);
      result &= chip_.l76k->ClearRxBufferData();
      result &= chip_.l76k->Sleep(true);
      if (!result) {
        chip_.l76k->Deinit();
      }

      status_.l76k.init_flag = result;
      if (result) {
        LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
      } else {
        LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
      }
      return result;
    }

  } else {
    bool result = true;
    result &=
        chip_.l76k->SetBaudRate(cpp_bus_driver::L76k::BaudRate::kBr115200Bps);
    result &= chip_.l76k->SetUpdateFrequency(
        cpp_bus_driver::L76k::UpdateFreq::kFreq5Hz);
    result &= chip_.l76k->ClearRxBufferData();
    result &= chip_.l76k->Sleep(true);
    if (!result) {
      chip_.l76k->Deinit();
    }

    status_.l76k.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
    } else {
      LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitL76k failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitPower() {
  CreateDrivers();
  if (power_initialized_) {
    return true;
  }
  bool power_enabled = true;
  power_enabled &= platform_hal_->SetGpioMode(
      gpio::power::kEnable3v3, cpp_bus_driver::PlatformHal::GpioMode::kOutput);
  power_enabled &= platform_hal_->GpioWrite(gpio::power::kEnable3v3, 1);
  if (!power_enabled) {
    return false;
  }
  platform_hal_->DelayMs(10);
  if (!InitLdoPower(3, 2500)) {
    return false;
  }
  if (!InitLdoPower(4, 3300)) {
    DeinitLdoPower(3);
    return false;
  }
  power_initialized_ = true;
  return true;
}

bool TDisplayP4Driver::InitScreenBacklight() {
  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return InitSy7200a();
    case device::ScreenType::kRm69a10:
      return true;
    default:
      return false;
  }
}

bool TDisplayP4Driver::DeinitEs8389() {
  bool result = true;
  result &= SetEs8389OperatingMode(Es8389OperatingMode::kSleep);

  if (es8389_input_codec_dev_ != nullptr) {
    esp_codec_dev_delete(es8389_input_codec_dev_);
    es8389_input_codec_dev_ = nullptr;
  }
  if (es8389_output_codec_dev_ != nullptr) {
    esp_codec_dev_delete(es8389_output_codec_dev_);
    es8389_output_codec_dev_ = nullptr;
  }
  if (es8389_codec_if_ != nullptr) {
    result &=
        (audio_codec_delete_codec_if(es8389_codec_if_) == ESP_CODEC_DEV_OK);
    es8389_codec_if_ = nullptr;
  }
  if (es8389_ctrl_if_ != nullptr) {
    result &= (audio_codec_delete_ctrl_if(es8389_ctrl_if_) == ESP_CODEC_DEV_OK);
    es8389_ctrl_if_ = nullptr;
  }
  if (es8389_data_if_ != nullptr) {
    result &= (audio_codec_delete_data_if(es8389_data_if_) == ESP_CODEC_DEV_OK);
    es8389_data_if_ = nullptr;
  }
  if (es8389_gpio_if_ != nullptr) {
    result &= (audio_codec_delete_gpio_if(es8389_gpio_if_) == ESP_CODEC_DEV_OK);
    es8389_gpio_if_ = nullptr;
  }
  if (bus_.es8389_i2s_bus != nullptr) {
    result &= bus_.es8389_i2s_bus->Deinit();
  }

  status_.es8389.init_flag = false;
  es8389_operating_mode_ = Es8389OperatingMode::kSleep;
  return result;
}

bool TDisplayP4Driver::DeinitIcm20948() {
  return true;
}

bool TDisplayP4Driver::DeinitLr2021() {
  bool result = true;
  if (status_.lr2021.init_flag && chip_.lr2021 != nullptr) {
    result &= SetLr2021OperatingMode(Lr2021OperatingMode::kSleep);
    result &= chip_.lr2021->Deinit(false);
  }
  status_.lr2021.init_flag = false;
  if (status_.xl9535.init_flag) {
    result &= chip_.xl9535->GpioWrite(
        gpio::xl9535::kLr2021Rst, device::xl9535::kResetAsserted);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021PowerEn, 0);
  }
  return result;
}

bool TDisplayP4Driver::DeinitPower() {
  bool result = true;
  result &= DeinitLdoPower(3);
  result &= DeinitLdoPower(4);
  if (platform_hal_ != nullptr) {
    // 关闭前需停止外设通信，将连接到断电外设的所有信号 IO 设为无上下拉的高阻态。
    // 否则 IO 反向供电可能导致断电不完全，使部分 I2C 设备下次初始化失败。
    // 电源使能脚需保持关闭电平；此处不自动配置其他 IO 的高阻态。
    result &= platform_hal_->GpioWrite(gpio::power::kEnable3v3, 0);
  }
  power_initialized_ = false;
  return result;
}

bool TDisplayP4Driver::DeinitScreenBacklight() {
  if (chip_.sy7200a == nullptr || !chip_.sy7200a->IsInitialized()) {
    status_.sy7200a.init_flag = false;
    return true;
  }

  const bool result = chip_.sy7200a->Deinit();
  status_.sy7200a.init_flag = chip_.sy7200a->IsInitialized();
  return result;
}

bool TDisplayP4Driver::IsAxp517Ready() const {
  return status_.axp517.init_flag && chip_.axp517 != nullptr;
}

bool TDisplayP4Driver::IsIcm20948Ready() const {
  return false;
}

bool TDisplayP4Driver::IsEs8389Ready() const {
  return status_.es8389.init_flag && es8389_input_codec_dev_ != nullptr &&
         es8389_output_codec_dev_ != nullptr;
}

bool TDisplayP4Driver::IsLr2021Ready() const {
  return status_.lr2021.init_flag && chip_.lr2021 != nullptr &&
         chip_.lr2021->initialized();
}

bool TDisplayP4Driver::IsScreenReady() const {
  if (bus_.screen_mipi_bus == nullptr ||
      bus_.screen_mipi_bus->device_handle() == nullptr) {
    return false;
  }

  switch (screen_type()) {
    case device::ScreenType::kHi8561:
      return IsHi8561Ready() && IsSy7200aReady();
    case device::ScreenType::kRm69a10:
      return IsRm69a10Ready();
    default:
      return false;
  }
}

bool TDisplayP4Driver::SetIcm20948Sleep(bool) {
  return false;
}

bool TDisplayP4Driver::SetEs8389OperatingMode(Es8389OperatingMode mode) {
  if (!status_.es8389.init_flag) {
    return mode == Es8389OperatingMode::kSleep && SetNs4150Enabled(false);
  }
  if (mode == es8389_operating_mode_) {
    return mode != Es8389OperatingMode::kSleep || SetNs4150Enabled(false);
  }

  bool result = true;
  if (mode == Es8389OperatingMode::kSleep) {
    result &=
        (esp_codec_dev_close(es8389_input_codec_dev_) == ESP_CODEC_DEV_OK);
    result &=
        (esp_codec_dev_close(es8389_output_codec_dev_) == ESP_CODEC_DEV_OK);
    result &= SetNs4150Enabled(false);
    es8389_operating_mode_ = Es8389OperatingMode::kSleep;
  } else {
    esp_codec_dev_sample_info_t output_sample_info = {
        .bits_per_sample = device::es8389::kBitsPerSample,
        .channel = device::es8389::kChannel,
        .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) |
                        ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1),
        .sample_rate = device::es8389::kSampleRate,
        .mclk_multiple = device::es8389::kMclkMultiple,
    };
    esp_codec_dev_sample_info_t input_sample_info = output_sample_info;
    const bool output_opened = esp_codec_dev_open(es8389_output_codec_dev_,
                                   &output_sample_info) == ESP_CODEC_DEV_OK;
    const bool input_opened =
        output_opened && esp_codec_dev_open(es8389_input_codec_dev_,
                             &input_sample_info) == ESP_CODEC_DEV_OK;
    result = output_opened && input_opened &&
             SetNs4150Enabled(true);
    if (!result) {
      if (input_opened) {
        esp_codec_dev_close(es8389_input_codec_dev_);
      }
      if (output_opened) {
        esp_codec_dev_close(es8389_output_codec_dev_);
      }
      SetNs4150Enabled(false);
    }
  }
  if (result && mode == Es8389OperatingMode::kActive) {
    es8389_operating_mode_ = mode;
  }
  return result;
}

bool TDisplayP4Driver::SetLr2021OperatingMode(Lr2021OperatingMode mode) {
  if (!IsLr2021Ready()) {
    if (mode == Lr2021OperatingMode::kSleep) {
      return true;
    }
    if (!InitLr2021()) {
      return false;
    }
  }

  lr20xx_status_t result = LR20XX_STATUS_ERROR;
  if (mode == Lr2021OperatingMode::kSleep) {
    const lr20xx_system_sleep_cfg_t sleep_config = {
        .is_clk_32k_enabled = false,
        .is_ram_retention_enabled = true,
    };
    result = chip_.lr2021->SetSleep(sleep_config) ? LR20XX_STATUS_OK
                                                  : LR20XX_STATUS_ERROR;
  } else if (chip_.lr2021->Wakeup()) {
    result = chip_.lr2021->Invoke(
        lr20xx_system_set_standby_mode, LR20XX_SYSTEM_STANDBY_MODE_RC);
  }

  if (result != LR20XX_STATUS_OK) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "LR2021 operating mode change failed (error code: %d)\n",
        static_cast<int>(result));
    return false;
  }
  return true;
}

bool TDisplayP4Driver::SetEsp32c5PowerEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, enabled ? 1 : 0);
}

bool TDisplayP4Driver::PrepareMinimalDriversForPowerOff() {
  bool result = true;
  if (IsXl9535Ready()) {
    result &= SetLedEnabled(false);
    result &= chip_.xl9535->Deinit(false);
    status_.xl9535.init_flag = false;
  }
  if (status_.axp517.init_flag && chip_.axp517 != nullptr) {
    result &= chip_.axp517->Deinit(false);
    status_.axp517.init_flag = false;
  }

  if (result) {
    result = DeinitPower();
  }

  minimal_drivers_initialized_ = false;
  return result;
}

bool TDisplayP4Driver::PrepareDriversForPowerOff() {
  if (!async_init_manager_.StopAndWait(kInitializationShutdownTimeoutMs)) {
    LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
        "Wait for asynchronous initialization before power off timed out\n");
    return false;
  }

  bool result = true;
  result &= DeinitScreenBacklight();
  result &= DeinitTouch();
  result &= DeinitScreen();
  result &= DeinitIcm20948();
  result &= DeinitAw86224();
  result &= DeinitEs8389();
  result &= DeinitLr2021();
  result &= DeinitL76k();
  result &= SetCameraPowerEnabled(false);
  result &= SetEsp32c5PowerEnabled(false);
  result &= DeinitSdmmc(false);
  if (!result) {
    return false;
  }

  if (IsSgm38121Ready()) {
    result &= chip_.sgm38121->Deinit(false);
    status_.sgm38121.init_flag = false;
  }
  if (IsXl9535Ready()) {
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kSdPowerEn, 0);
    result &= chip_.xl9535->GpioWrite(gpio::xl9535::kGpsRst, 0);
  }
  return result && PrepareMinimalDriversForPowerOff();
}

bool TDisplayP4Driver::EnterEsp32c5DownloadMode() {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
    return false;
  }

  bool result = true;

  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 0);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 0);
  platform_hal_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5En, 1);
  platform_hal_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(gpio::xl9535::kEsp32c5Boot, 1);

  if (!result) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "EnterEsp32c5DownloadMode failed\n");
  }
  return result;
}

bool TDisplayP4Driver::SetLedEnabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kLed, enabled ? 1 : 0);
}

void TDisplayP4Driver::CreateDrivers() {
  if (platform_hal_ != nullptr) {
    return;
  }
  platform_hal_ = std::make_unique<cpp_bus_driver::PlatformHal>();

  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      gpio::sgm38121::kSda, gpio::sgm38121::kScl, I2C_NUM_0);
  bus_.xl9535_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      gpio::xl9535::kSda, gpio::xl9535::kScl, I2C_NUM_1);

  bus_.axp517_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9535_i2c_bus);
  bus_.hi8561_i2c_touch_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
  bus_.gt9895_i2c_touch_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
  bus_.aw86224_i2c_bus =
      std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.xl9535_i2c_bus);

  bus_.es8389_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
      gpio::es8389::kAdcData, gpio::es8389::kDacData, gpio::es8389::kWsLrck,
      gpio::es8389::kBclk, gpio::es8389::kMclk, i2s_port_t::I2S_NUM_0,
      cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
      cpp_bus_driver::HardwareI2s::I2sMode::kStd,
      i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);

  bus_.lr2021_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(gpio::lr2021::kMosi,
          gpio::lr2021::kSclk, gpio::lr2021::kMiso, SPI2_HOST, 0);

  bus_.l76k_uart_bus = std::make_shared<cpp_bus_driver::HardwareUart>(
      gpio::l76k::kTx, gpio::l76k::kRx, UART_NUM_1);

  chip_.axp517 = std::make_unique<cpp_bus_driver::Axp517>(
      bus_.axp517_i2c_bus, device::axp517::kI2cAddress);
  chip_.xl9535 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9535_i2c_bus, device::xl9535::kI2cAddress);
  chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
      bus_.sgm38121_i2c_bus, device::sgm38121::kI2cAddress);
  chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      bus_.aw86224_i2c_bus, device::aw86224::kI2cAddress);
  chip_.hi8561_touch = std::make_unique<cpp_bus_driver::Hi8561Touch>(
      bus_.hi8561_i2c_touch_bus, device::hi8561::kTouchI2cAddress);
  chip_.gt9895 = std::make_unique<cpp_bus_driver::Gt9895>(
      bus_.gt9895_i2c_touch_bus, device::gt9895::kI2cAddress);
  chip_.sy7200a = std::make_unique<cpp_bus_driver::Pwm>(gpio::sy7200a::kEn);
  chip_.l76k = std::make_unique<cpp_bus_driver::L76k>(
      bus_.l76k_uart_bus, [this](bool value) {
        return IsXl9535Ready() &&
               chip_.xl9535->GpioWrite(
                   gpio::xl9535::kGpsWakeUp, static_cast<uint8_t>(value));
      });
  chip_.lr2021 = std::make_unique<usp_cpp_bus_driver::Lr20xx>(
      bus_.lr2021_spi_bus, gpio::lr2021::kBusy, gpio::lr2021::kCs,
      [this](bool released) {
        return IsXl9535Ready() &&
               chip_.xl9535->GpioWrite(gpio::xl9535::kLr2021Rst,
                   released ? device::xl9535::kResetReleased
                            : device::xl9535::kResetAsserted);
      });
}

bool TDisplayP4Driver::InitDrivers(InitMode mode) {
  if (!InitMinimalDrivers() || !InitSgm38121()) {
    return false;
  }
  bool result = true;
  async_init_manager_.Reset();

  if (mode == InitMode::kAsync) {
    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4Driver*>(arg);
          const bool screen_initialized = self->InitScreen();

          if (screen_initialized) {
            self->InitTouch();
            self->InitScreenBacklight();
          }

          self->async_init_manager_.FinishTask();
        },
        "InitScreenTask", 8192, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitAw86224();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitAw86224Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitLr2021();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitLr2021Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitL76k();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitL76kTask", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TDisplayP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested() &&
              !self->InitEs8389()) {
            self->SetNs4150Enabled(false);
          }
          self->async_init_manager_.FinishTask();
        },
        "InitEs8389Task", 4096, this, 3);

  } else {
    const bool screen_initialized = InitScreen();
    result &= screen_initialized;
    if (screen_initialized) {
      result &= InitTouch();
      result &= InitScreenBacklight();
    }

    // result &= InitIcm20948();

    result &= InitAw86224();
    result &= InitLr2021();
    result &= InitL76k();

    result &= InitEs8389();
  }

  return result;
}

bool TDisplayP4Driver::InitMinimalDrivers() {
  if (minimal_drivers_initialized_) {
    return true;
  }
  if (!InitPower() || !InitAxp517() || !InitXl9535()) {
    return false;
  }
  minimal_drivers_initialized_ = true;
  return true;
}

bool TDisplayP4Driver::SetNs4150Enabled(bool enabled) {
  if (!status_.xl9535.init_flag) {
    return !enabled;
  }
  return chip_.xl9535->GpioWrite(gpio::xl9535::kNs4150En, enabled ? 1 : 0);
}

void TDisplayP4Driver::ResetScreenBacklightStatus() {
  status_.sy7200a.init_flag = false;
}

}  // namespace lilygo_device_driver
