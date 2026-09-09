/*
 * @Description: T-Glasses-P4 板级设备驱动实现
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:58:49
 * @License: GPL 3.0
 */
#include "device/t_glasses_p4/driver.h"

#include <cstdio>

#include "../../core/logger.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// 其余外围恢复时再启用这些依赖。
// #include "driver/gpio.h"
// #include "driver/sdspi_host.h"
// #include "driver/spi_master.h"

namespace lilygo_device_driver {
namespace gpio = t_glasses_p4::gpio;
namespace device = t_glasses_p4::device;
namespace {

// 设备正常显示时的镜像模式。
constexpr auto kDefaultScreenMirror =
    cpp_bus_driver::S023msafjf10111e1::MirrorMode::kHorizontal;

constexpr device::ScreenInfo kScreenInfo = {
    .type = device::ScreenType::kS023msafjf10111e1,
    .name = "s023msafjf10111e1",
    .width = device::screen::kWidth,
    .height = device::screen::kHeight,
    .bits_per_pixel = device::screen::kBitsPerPixel,
    .pixel_format = GetRgbPixelFormatName(device::screen::kBitsPerPixel),
    .mipi_dsi_dpi_clk_mhz = device::screen::kMipiDsiDpiClkMhz,
    .mipi_dsi_hsync = device::screen::kMipiDsiHsync,
    .mipi_dsi_hbp = device::screen::kMipiDsiHbp,
    .mipi_dsi_hfp = device::screen::kMipiDsiHfp,
    .mipi_dsi_vsync = device::screen::kMipiDsiVsync,
    .mipi_dsi_vbp = device::screen::kMipiDsiVbp,
    .mipi_dsi_vfp = device::screen::kMipiDsiVfp,
    .data_lane_num = device::screen::kDataLaneNum,
    .lane_bit_rate_mbps = device::screen::kLaneBitRateMbps,
};

// 旧版 CreateDrivers 的屏幕信息指针配套参考。
// constexpr const device::ScreenInfo* kDefaultScreenInfo = &kScreenInfo;
constexpr uint32_t kInitializationShutdownTimeoutMs = 5 * 1000;
constexpr uint8_t kLr2021ExpectedVersionMajor = 0x01;
constexpr uint8_t kLr2021ExpectedVersionMinor = 0x18;

}  // namespace

TGlassesP4Driver& TGlassesP4Driver::GetInstance() {
  static TGlassesP4Driver* instance = new TGlassesP4Driver();
  return *instance;
}

const device::ScreenInfo& TGlassesP4Driver::screen_info() const {
  return kScreenInfo;
}

device::ScreenType TGlassesP4Driver::screen_type() const {
  return screen_info().type;
}

void TGlassesP4Driver::CreateDrivers() {
  if (platform_hal_ != nullptr) {
    return;
  }
  platform_hal_ = std::make_unique<cpp_bus_driver::PlatformHal>();
  bus_.bq25896_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      gpio::bq25896::kSda, gpio::bq25896::kScl, I2C_NUM_1);
  bus_.screen_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      gpio::s023msafjf10111e1::kSda, gpio::s023msafjf10111e1::kScl, I2C_NUM_0);
  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
      gpio::sgm38121::kSda, gpio::sgm38121::kScl, LP_I2C_NUM_0);
  chip_.bq25896 = std::make_unique<cpp_bus_driver::Bq2589x>(
      bus_.bq25896_i2c_bus, device::bq25896::kI2cAddress,
      cpp_bus_driver::Bq2589x::ChipModel::kBq25896);
  chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
      bus_.sgm38121_i2c_bus, device::sgm38121::kI2cAddress);
  chip_.s023msafjf10111e1 = std::make_unique<cpp_bus_driver::S023msafjf10111e1>(
      bus_.screen_i2c_bus, device::s023msafjf10111e1::kI2cAddress,
      gpio::s023msafjf10111e1::kRst);

  bus_.es8389_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
      gpio::es8389::kAdcData, gpio::es8389::kDacData, gpio::es8389::kWsLrck,
      gpio::es8389::kBclk, gpio::es8389::kMclk, i2s_port_t::I2S_NUM_0,
      cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
      cpp_bus_driver::HardwareI2s::I2sMode::kStd,
      i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);

  // 新板外围对象创建参考，待硬件完善后连同头文件成员一起恢复。
  // bus_.bq27220_i2c_bus =
  //     std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.bq25896_i2c_bus);
  // AW86224 与 SGM38121 共用引脚，恢复后共享 LP I2C 总线。
  // bus_.aw86224_i2c_bus =
  //     std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
  bus_.lr2021_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(gpio::lr2021::kMosi,
          gpio::lr2021::kSclk, gpio::lr2021::kMiso, SPI2_HOST, 0);
  // chip_.bq27220 = std::make_unique<cpp_bus_driver::Bq27220>(
  //     bus_.bq27220_i2c_bus, device::bq27220::kI2cAddress);
  // chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
  //     bus_.aw86224_i2c_bus, device::aw86224::kI2cAddress);
  chip_.lr2021 =
      std::make_unique<usp_cpp_bus_driver::Lr20xx>(bus_.lr2021_spi_bus,
          gpio::lr2021::kBusy, gpio::lr2021::kCs, [this](bool level) {
            return platform_hal_->GpioWrite(gpio::lr2021::kRst, level);
          });
}

bool TGlassesP4Driver::Init(InitMode mode) {
  CreateDrivers();
  const int64_t start_time_us = platform_hal_->GetSystemTimeUs();
  const bool result = InitDrivers(mode);
  const int64_t elapsed_time_us =
      platform_hal_->GetSystemTimeUs() - start_time_us;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      "TGlassesP4Driver init (mode: %s, result: %s, elapsed: "
      "%lld ms)\n",
      mode == InitMode::kAsync ? "async" : "sync",
      result ? (mode == InitMode::kAsync ? "tasks scheduled" : "success")
             : "failed",
      static_cast<long long>(elapsed_time_us / 1000));
  return result;
}

bool TGlassesP4Driver::InitMinimal() {
  CreateDrivers();
  // 初始化失败保留公共电源，允许后续重试。
  return InitMinimalDrivers();
}

bool TGlassesP4Driver::InitMinimalDrivers() {
  if (minimal_drivers_initialized_) {
    return true;
  }
  if (!InitPower() || !InitBq25896()) {
    return false;
  }
  minimal_drivers_initialized_ = true;
  return true;
}

bool TGlassesP4Driver::InitDrivers(InitMode mode) {
  if (mode != InitMode::kSync && mode != InitMode::kAsync) {
    return false;
  }
  if (!InitMinimalDrivers()) {
    return false;
  }
  bool result = InitSgm38121();
  async_init_manager_.Reset();
  if (mode == InitMode::kAsync) {
    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitScreen();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitScreenTask", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitEs8389();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitEs8389Task", 4096, this, 3);

    result &= async_init_manager_.StartTask(
        [](void* arg) {
          auto* self = static_cast<TGlassesP4Driver*>(arg);
          if (!self->async_init_manager_.stop_requested()) {
            self->InitLr2021();
          }
          self->async_init_manager_.FinishTask();
        },
        "InitLr2021Task", 4096, this, 3);

    // 未接入外围的异步初始化参考。
    // result &= async_init_manager_.StartTask(
    //     [](void* arg) {
    //       auto* self = static_cast<TGlassesP4Driver*>(arg);
    //       if (!self->async_init_manager_.stop_requested()) {
    //         self->InitBq27220();
    //         self->InitAw86224();
    //       }
    //       self->async_init_manager_.FinishTask();
    //     },
    //     "PeripheralTask", 4096, this, 3);
  } else {
    result &= InitScreen();
    // result &= InitBq27220();
    // result &= InitAw86224();
    result &= InitLr2021();
    result &= InitEs8389();
  }
  return result;
}

bool TGlassesP4Driver::InitBq25896() {
  if (IsBq25896Ready()) {
    return true;
  }
  if (!power_initialized_ || chip_.bq25896 == nullptr) {
    return false;
  }
  const bool result = chip_.bq25896->Init();
  if (!result) {
    chip_.bq25896->Deinit(false);
  }
  status_.bq25896.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitBq25896 success\n" : "InitBq25896 failed\n");
  return result;
}

bool TGlassesP4Driver::InitSgm38121() {
  if (IsSgm38121Ready()) {
    return true;
  }
  if (!power_initialized_ || chip_.sgm38121 == nullptr) {
    return false;
  }
  if (!chip_.sgm38121->Init()) {
    chip_.sgm38121->Deinit(false);
    LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1,
      cpp_bus_driver::Sgm38121::Status::kOff);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2,
      cpp_bus_driver::Sgm38121::Status::kOff);

  // AVDD1 为屏幕、摄像头和外部时钟共享的 1.8 V 电源。
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 3000);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
  result &= chip_.sgm38121->SetOutputVoltage(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
#endif
  if (result) {
    result = chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1,
        cpp_bus_driver::Sgm38121::Status::kOn);
  }
  if (result) {
    platform_hal_->DelayMs(10);
  } else {
    chip_.sgm38121->Deinit(false);
  }
  status_.sgm38121.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitSgm38121 success\n" : "InitSgm38121 failed\n");
  return result;
}

bool TGlassesP4Driver::InitS023msafjf10111e1() {
  if (IsS023msafjf10111e1Ready()) {
    return true;
  }
  if (!power_initialized_ || !IsSgm38121Ready() ||
      chip_.s023msafjf10111e1 == nullptr || !chip_.s023msafjf10111e1->Init()) {
    LogMessage(
        LogLevel::kError, __FILE__, __LINE__, "InitS023msafjf10111e1 failed\n");
    return false;
  }
  bool result = chip_.s023msafjf10111e1->SetBrightnessGain(0);
  // 设置屏幕的默认显示方向。
  result &= chip_.s023msafjf10111e1->SetMirror(kDefaultScreenMirror);
  result &= chip_.s023msafjf10111e1->SetPixelShift(0, 0);
  result &= chip_.s023msafjf10111e1->SetBistEnabled(false);
  if (!result) {
    chip_.s023msafjf10111e1->Deinit(false);
  }
  status_.s023msafjf10111e1.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitS023msafjf10111e1 success\n"
             : "InitS023msafjf10111e1 failed\n");
  return result;
}

// 外围函数空壳，后续硬件完善后再接入对应驱动。
bool TGlassesP4Driver::InitBq27220() { return false; }

bool TGlassesP4Driver::InitEs8389() {
  if (IsEs8389Ready()) {
    return true;
  }
  if (!power_initialized_ || bus_.es8389_i2s_bus == nullptr ||
      bus_.sgm38121_i2c_bus == nullptr ||
      bus_.sgm38121_i2c_bus->bus_handle() == nullptr) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitEs8389 failed (power or bus not ready)\n");
    return false;
  }
  audio_codec_i2c_cfg_t i2c_cfg = {
      .port = static_cast<uint8_t>(LP_I2C_NUM_0),
      .addr = static_cast<uint8_t>(device::es8389::kI2cAddress << 1),
      .bus_handle = bus_.sgm38121_i2c_bus->bus_handle(),
  };
  es8389_ctrl_if_ = audio_codec_new_i2c_ctrl(&i2c_cfg);
  if (es8389_ctrl_if_ == nullptr ||
      !bus_.es8389_i2s_bus->Init(
          static_cast<i2s_mclk_multiple_t>(device::es8389::kMclkMultiple),
          device::es8389::kSampleRate,
          static_cast<i2s_data_bit_width_t>(device::es8389::kBitsPerSample))) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitEs8389 failed (I2C control or I2S initialization)\n");
    return false;
  }
  audio_codec_i2s_cfg_t i2s_cfg = {
      .port = static_cast<uint8_t>(bus_.es8389_i2s_bus->port()),
      .rx_handle = bus_.es8389_i2s_bus->rx_handle(),
      .tx_handle = bus_.es8389_i2s_bus->tx_handle(),
      .clk_src = static_cast<int>(I2S_CLK_SRC_DEFAULT),
  };
  es8389_data_if_ = audio_codec_new_i2s_data(&i2s_cfg);
  es8389_gpio_if_ = audio_codec_new_gpio();
  if (es8389_data_if_ == nullptr || es8389_gpio_if_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitEs8389 failed (data or GPIO interface)\n");
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
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitEs8389 failed (codec interface)\n");
    return false;
  }
  esp_codec_dev_cfg_t output_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_OUT,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_output_codec_dev_ = esp_codec_dev_new(&output_dev_cfg);
  esp_codec_dev_cfg_t input_dev_cfg = {
      .dev_type = ESP_CODEC_DEV_TYPE_IN,
      .codec_if = es8389_codec_if_,
      .data_if = es8389_data_if_,
  };
  es8389_input_codec_dev_ = esp_codec_dev_new(&input_dev_cfg);
  if (es8389_output_codec_dev_ == nullptr ||
      es8389_input_codec_dev_ == nullptr) {
    DeinitEs8389();
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitEs8389 failed (codec device)\n");
    return false;
  }
  status_.es8389.init_flag = true;
  bool result = SetEs8389OperatingMode(Es8389OperatingMode::kActive);
  if (result) {
    result &= esp_codec_dev_set_out_vol(es8389_output_codec_dev_, 100) ==
              ESP_CODEC_DEV_OK;
    result &= esp_codec_dev_set_in_gain(es8389_input_codec_dev_, 20.0f) ==
              ESP_CODEC_DEV_OK;
  }
  result &= SetEs8389OperatingMode(Es8389OperatingMode::kSleep);
  if (!result) {
    DeinitEs8389();
  }
  status_.es8389.init_flag = result;
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitEs8389 success\n" : "InitEs8389 failed\n");
  return result;
}

bool TGlassesP4Driver::InitAw86224() { return false; }

bool TGlassesP4Driver::InitBhi260ap() { return false; }

bool TGlassesP4Driver::InitBmm350() { return false; }

bool TGlassesP4Driver::InitLr2021() {
  if (IsLr2021Ready()) {
    return true;
  }
  if (!power_initialized_ || chip_.lr2021 == nullptr) {
    return false;
  }
  bool result = platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
  result &= platform_hal_->SetGpioMode(
      gpio::lr2021::kRst, cpp_bus_driver::PlatformHal::GpioMode::kOutput);
  result &= platform_hal_->SetGpioMode(
      gpio::lr2021::kInt, cpp_bus_driver::PlatformHal::GpioMode::kInput);
  // 模块辅助信号由无线芯片驱动，主控保持无上下拉输入。
  result &= platform_hal_->SetGpioMode(
      gpio::lr2021::kDio3, cpp_bus_driver::PlatformHal::GpioMode::kInput);
  if (!result || !chip_.lr2021->Init(device::lr2021::kSpiFrequencyHz)) {
    chip_.lr2021->Deinit(false);
    platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "InitLr2021 transport failed\n");
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
    platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
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
  result = true;
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
    platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
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

bool TGlassesP4Driver::InitPower() {
  if (power_initialized_) {
    return true;
  }
  if (platform_hal_ == nullptr) {
    return false;
  }
  if (!InitLdoPower(4, 3300)) {
    return false;
  }
  if (!InitLdoPower(3, 2500)) {
    DeinitLdoPower(4);
    return false;
  }
  if (!platform_hal_->SetGpioMode(gpio::power::kEnable3v3,
          cpp_bus_driver::PlatformHal::GpioMode::kOutput) ||
      !platform_hal_->GpioWrite(gpio::power::kEnable3v3, 1)) {
    DeinitLdoPower(3);
    DeinitLdoPower(4);
    return false;
  }
  platform_hal_->DelayMs(200);
  power_initialized_ = true;
  return true;
}

bool TGlassesP4Driver::InitScreen() {
  if (IsScreenReady()) {
    return true;
  }
  if (!InitS023msafjf10111e1()) {
    return false;
  }
  const auto& screen = screen_info();
  bus_.screen_mipi_bus = std::make_shared<cpp_bus_driver::HardwareMipi>(
      screen.width, screen.height, screen.mipi_dsi_hsync, screen.mipi_dsi_hbp,
      screen.mipi_dsi_hfp, screen.mipi_dsi_vsync, screen.mipi_dsi_vbp,
      screen.mipi_dsi_vfp, screen.data_lane_num,
      cpp_bus_driver::HardwareMipi::ColorFormat::kRgb888);
  const bool result = bus_.screen_mipi_bus->Init(screen.mipi_dsi_dpi_clk_mhz,
                          screen.lane_bit_rate_mbps) &&
                      bus_.screen_mipi_bus->StartTransmit();
  if (!result) {
    DeinitScreen();
  }
  LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
      result ? "InitScreen success\n" : "InitScreen failed\n");
  return result;
}

bool TGlassesP4Driver::InitSdmmc(const char* base_path, int max_freq_khz) {
  if (base_path == nullptr || base_path[0] == '\0' || max_freq_khz <= 0) {
    return false;
  }
  if (sd_card_.IsMounted()) {
    return sd_card_.base_path() == base_path && IsSdmmcReady();
  }
  if (!power_initialized_) {
    return false;
  }

  SdCard::SdmmcConfig config;
  config.host.slot = SDMMC_HOST_SLOT_0;
  config.host.max_freq_khz = max_freq_khz;
  config.slot.width = 4;
  config.slot.clk = static_cast<gpio_num_t>(gpio::sd::kSdioClk);
  config.slot.cmd = static_cast<gpio_num_t>(gpio::sd::kSdioCmd);
  config.slot.d0 = static_cast<gpio_num_t>(gpio::sd::kSdioD0);
  config.slot.d1 = static_cast<gpio_num_t>(gpio::sd::kSdioD1);
  config.slot.d2 = static_cast<gpio_num_t>(gpio::sd::kSdioD2);
  config.slot.d3 = static_cast<gpio_num_t>(gpio::sd::kSdioD3);
  config.slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

  const bool result = sd_card_.InitSdmmc(base_path, config);
  status_.sd_card.init_flag = sd_card_.IsMounted();
  return result;
}

bool TGlassesP4Driver::DeinitEs8389() {
  bool result = SetEs8389OperatingMode(Es8389OperatingMode::kSleep);
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
        audio_codec_delete_codec_if(es8389_codec_if_) == ESP_CODEC_DEV_OK;
    es8389_codec_if_ = nullptr;
  }
  if (es8389_ctrl_if_ != nullptr) {
    result &=
        audio_codec_delete_ctrl_if(es8389_ctrl_if_) == ESP_CODEC_DEV_OK;
    es8389_ctrl_if_ = nullptr;
  }
  if (es8389_data_if_ != nullptr) {
    result &=
        audio_codec_delete_data_if(es8389_data_if_) == ESP_CODEC_DEV_OK;
    es8389_data_if_ = nullptr;
  }
  if (es8389_gpio_if_ != nullptr) {
    result &=
        audio_codec_delete_gpio_if(es8389_gpio_if_) == ESP_CODEC_DEV_OK;
    es8389_gpio_if_ = nullptr;
  }
  if (bus_.es8389_i2s_bus != nullptr) {
    result &= bus_.es8389_i2s_bus->Deinit();
  }
  status_.es8389.init_flag = false;
  es8389_operating_mode_ = Es8389OperatingMode::kSleep;
  return result;
}

bool TGlassesP4Driver::DeinitLr2021() {
  bool result = true;
  if (IsLr2021Ready()) {
    result &= SetLr2021OperatingMode(Lr2021OperatingMode::kSleep);
    result &= chip_.lr2021->Deinit(false);
    result &= platform_hal_->GpioWrite(gpio::lr2021::kRst, 0);
  }
  status_.lr2021.init_flag = false;
  return result;
}

bool TGlassesP4Driver::DeinitSdmmc() {
  bool result = sd_card_.Deinit();
  status_.sd_card.init_flag = sd_card_.IsMounted();
  return result;
}

bool TGlassesP4Driver::DeinitPower() {
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

bool TGlassesP4Driver::DeinitScreen() {
  bool result = true;
  if (IsS023msafjf10111e1Ready()) {
    result &= chip_.s023msafjf10111e1->SetBrightnessGain(0);
  }
  if (bus_.screen_mipi_bus != nullptr) {
    result &= bus_.screen_mipi_bus->Deinit();
    bus_.screen_mipi_bus.reset();
  }
  if (IsS023msafjf10111e1Ready()) {
    result &= chip_.s023msafjf10111e1->Deinit(false);
  }
  status_.s023msafjf10111e1.init_flag = false;
  return result;
}

bool TGlassesP4Driver::IsEs8389Ready() const {
  return status_.es8389.init_flag && es8389_input_codec_dev_ != nullptr &&
         es8389_output_codec_dev_ != nullptr;
}

bool TGlassesP4Driver::IsSdmmcReady() const {
  return status_.sd_card.init_flag && sd_card_.IsReady();
}

bool TGlassesP4Driver::IsBq25896Ready() const {
  return status_.bq25896.init_flag && chip_.bq25896 != nullptr;
}

bool TGlassesP4Driver::IsSgm38121Ready() const {
  return status_.sgm38121.init_flag && chip_.sgm38121 != nullptr;
}

bool TGlassesP4Driver::IsS023msafjf10111e1Ready() const {
  return status_.s023msafjf10111e1.init_flag &&
         chip_.s023msafjf10111e1 != nullptr;
}

bool TGlassesP4Driver::IsLr2021Ready() const {
  return status_.lr2021.init_flag && chip_.lr2021 != nullptr;
}

bool TGlassesP4Driver::IsScreenReady() const {
  return IsS023msafjf10111e1Ready() && bus_.screen_mipi_bus != nullptr &&
         bus_.screen_mipi_bus->device_handle() != nullptr;
}

bool TGlassesP4Driver::SetEs8389OperatingMode(Es8389OperatingMode mode) {
  if (!IsEs8389Ready()) {
    return mode == Es8389OperatingMode::kSleep;
  }
  if (mode == es8389_operating_mode_) {
    return true;
  }
  if (mode != Es8389OperatingMode::kSleep &&
      mode != Es8389OperatingMode::kActive) {
    return false;
  }
  if (mode == Es8389OperatingMode::kSleep) {
    bool result =
        esp_codec_dev_close(es8389_input_codec_dev_) == ESP_CODEC_DEV_OK;
    result &=
        esp_codec_dev_close(es8389_output_codec_dev_) == ESP_CODEC_DEV_OK;
    if (result) {
      es8389_operating_mode_ = mode;
    }
    return result;
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
  if (esp_codec_dev_open(es8389_output_codec_dev_, &output_sample_info) !=
      ESP_CODEC_DEV_OK) {
    return false;
  }
  if (esp_codec_dev_open(es8389_input_codec_dev_, &input_sample_info) !=
      ESP_CODEC_DEV_OK) {
    esp_codec_dev_close(es8389_output_codec_dev_);
    return false;
  }
  es8389_operating_mode_ = mode;
  return true;
}

bool TGlassesP4Driver::SetLr2021OperatingMode(Lr2021OperatingMode mode) {
  if (!IsLr2021Ready()) {
    return mode == Lr2021OperatingMode::kSleep;
  }
  switch (mode) {
    case Lr2021OperatingMode::kStandby:
      return chip_.lr2021->Wakeup() &&
             chip_.lr2021->Invoke(lr20xx_system_set_standby_mode,
                 LR20XX_SYSTEM_STANDBY_MODE_RC) == LR20XX_STATUS_OK;
    case Lr2021OperatingMode::kSleep: {
      const lr20xx_system_sleep_cfg_t sleep_config = {
          .is_clk_32k_enabled = false,
          .is_ram_retention_enabled = true,
      };
      return chip_.lr2021->SetSleep(sleep_config);
    }
    default:
      return false;
  }
}

bool TGlassesP4Driver::SetEsp32c5PowerEnabled(bool /*enabled*/) {
  return false;
}

bool TGlassesP4Driver::SetCameraPowerEnabled(bool enabled) {
  if (!IsSgm38121Ready()) {
    return !enabled;
  }
  const auto status = enabled ? cpp_bus_driver::Sgm38121::Status::kOn
                              : cpp_bus_driver::Sgm38121::Status::kOff;
  bool result = true;
  // AVDD1 与屏幕共享，摄像头电源切换只操作独立电源通道。
#if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, status);
#elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710) || \
    defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kDvdd1, status);
  result &= chip_.sgm38121->SetChannelStatus(
      cpp_bus_driver::Sgm38121::Channel::kAvdd2, status);
#else
  return false;
#endif
  return result;
}

bool TGlassesP4Driver::PrepareMinimalDriversForPowerOff() {
  bool result = true;
  if (IsBq25896Ready()) {
    // 仅解除驱动和总线初始化，保留充电配置，不断开电池供电。
    result &= chip_.bq25896->Deinit(false);
  }
  status_.bq25896.init_flag = false;
  result &= DeinitPower();
  minimal_drivers_initialized_ = false;
  return result;
}

bool TGlassesP4Driver::PrepareDriversForPowerOff() {
  if (!async_init_manager_.StopAndWait(kInitializationShutdownTimeoutMs)) {
    LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
        "Wait for asynchronous initialization before power off timed out\n");
    return false;
  }
  bool result = true;
  result &= DeinitScreen();
  // 外围关机参考，恢复外围后应在关闭公共电源前执行。
  // result &= DeinitAw86224();
  result &= DeinitEs8389();
  result &= DeinitLr2021();
  result &= SetCameraPowerEnabled(false);
  result &= DeinitSdmmc();
  // result &= SetEsp32c5PowerEnabled(false);
  // if (IsBq27220Ready()) {
  //   result &= chip_.bq27220->Deinit(false);
  //   status_.bq27220.init_flag = false;
  // }
  if (IsSgm38121Ready()) {
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1,
        cpp_bus_driver::Sgm38121::Status::kOff);
    result &= chip_.sgm38121->Deinit(false);
  }
  status_.sgm38121.init_flag = false;
  result &= PrepareMinimalDriversForPowerOff();
  return result;
}

bool TGlassesP4Driver::SetScreenMirror(bool horizontal, bool vertical) {
  using Screen = cpp_bus_driver::S023msafjf10111e1;
  if (!IsS023msafjf10111e1Ready()) {
    return false;
  }
  const bool initial_horizontal =
      kDefaultScreenMirror == Screen::MirrorMode::kHorizontal ||
      kDefaultScreenMirror == Screen::MirrorMode::kHorizontalVertical;
  const bool initial_vertical =
      kDefaultScreenMirror == Screen::MirrorMode::kVertical ||
      kDefaultScreenMirror == Screen::MirrorMode::kHorizontalVertical;
  const bool target_horizontal = initial_horizontal != horizontal;
  const bool target_vertical = initial_vertical != vertical;
  const auto target =
      target_horizontal
          ? (target_vertical ? Screen::MirrorMode::kHorizontalVertical
                             : Screen::MirrorMode::kHorizontal)
          : (target_vertical ? Screen::MirrorMode::kVertical
                             : Screen::MirrorMode::kOff);
  if (!chip_.s023msafjf10111e1->SetMirror(target)) {
    return false;
  }
  Screen::MirrorMode actual;
  if (!chip_.s023msafjf10111e1->GetMirror(&actual)) {
    return false;
  }
  LogMessage(LogLevel::kDebug, __FILE__, __LINE__,
      "Screen mirror: initial=%d, target=%d, readback=%d, RSMX=%d, RSMY=%d\n",
      static_cast<int>(kDefaultScreenMirror), static_cast<int>(target),
      static_cast<int>(actual), target_horizontal, target_vertical);
  if (actual != target) {
    LogMessage(LogLevel::kError, __FILE__, __LINE__,
        "SetScreenMirror readback mismatch (target: %d, actual: %d)\n",
        static_cast<int>(target), static_cast<int>(actual));
    return false;
  }
  return true;
}
// 新板外围实现参考：当前整段停用，尚未完成本板硬件验证。
// 恢复时须同步恢复头文件成员和依赖，并替换下方同名 false 空壳。
// BQ27220 保留现有电池配置；ES8389 不套用 Air 板特有的功放和极性设置。
// bool TGlassesP4Driver::InitBq27220() {
//   CreateDrivers();
//   if (IsBq27220Ready()) {
//     return true;
//   }
//   if (!InitPower()) {
//     return false;
//   }
//   const bool result = chip_.bq27220->Init();
//   if (!result) {
//     chip_.bq27220->Deinit(false);
//   }
//   status_.bq27220.init_flag = result;
//   LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
//       "InitBq27220 %s\n", result ? "success" : "failed");
//   return result;
// }
//
//
//
//
//
// bool TGlassesP4Driver::SetEsp32c5PowerEnabled(bool enabled) {
//   CreateDrivers();
//   bool result = platform_hal_->SetGpioMode(
//       gpio::esp32c5::kEn, cpp_bus_driver::PlatformHal::GpioMode::kOutput);
//   result &= platform_hal_->GpioWrite(gpio::esp32c5::kEn, 0);
//   result &= platform_hal_->SetGpioMode(
//       gpio::esp32c5::kPowerEn,
//       cpp_bus_driver::PlatformHal::GpioMode::kOutput);
//   result &= platform_hal_->GpioWrite(gpio::esp32c5::kPowerEn, enabled);
//   if (enabled && result) {
//     platform_hal_->DelayMs(10);
//     result = platform_hal_->GpioWrite(gpio::esp32c5::kEn, 1);
//   }
//   if (!result) {
//     platform_hal_->GpioWrite(gpio::esp32c5::kEn, 0);
//     platform_hal_->GpioWrite(gpio::esp32c5::kPowerEn, 0);
//   }
//   return result;
// }
//
// bool TGlassesP4Driver::InitAw86224() {
//   if (IsAw86224Ready()) {
//     return true;
//   }
//   if (!InitMinimal()) {
//     return false;
//   }
//   if (!chip_.aw86224->Init(device::aw86224::kI2cFrequencyHz)) {
//     status_.aw86224.init_flag = false;
//     status_.aw86224.ram_waveform_selection =
//         cpp_bus_driver::Aw862xx::RamWaveformSelection();
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAw86224 failed\n");
//     return false;
//   }
//
//   cpp_bus_driver::Aw862xx::RamWaveformSelection selection;
//   bool result = chip_.aw86224->InitRamModeByF0(selection);
//   if (result) {
//     result = chip_.aw86224->StopRamPlaybackWaveform();
//   }
//   if (!result) {
//     chip_.aw86224->Deinit(false);
//   }
//
//   status_.aw86224.init_flag = result;
//   status_.aw86224.ram_waveform_selection =
//       result ? selection : cpp_bus_driver::Aw862xx::RamWaveformSelection();
//   if (result) {
//     LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitAw86224 success\n");
//   } else {
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitAw86224 failed\n");
//   }
//   return result;
// }
//
//
// bool TGlassesP4Driver::DeinitAw86224() {
//   bool result = true;
//   if (status_.aw86224.init_flag && chip_.aw86224 != nullptr) {
//     result &= chip_.aw86224->StopRamPlaybackWaveform();
//     result &= chip_.aw86224->Deinit(false);
//   }
//   status_.aw86224.init_flag = false;
//   status_.aw86224.ram_waveform_selection = {};
//   return result;
// }
//
//
// bool TGlassesP4Driver::IsBq27220Ready() const {
//   return status_.bq27220.init_flag && chip_.bq27220 != nullptr;
// }
//
// bool TGlassesP4Driver::IsAw86224Ready() const {
//   return status_.aw86224.init_flag && chip_.aw86224 != nullptr;
// }
//
//
//
// bool TGlassesP4Driver::SetAw86224Standby() {
//   return !IsAw86224Ready() || chip_.aw86224->StopRamPlaybackWaveform();
// }
//

// SD 卡 SPI 模式参考，后续需要时再接入。
//
// bool TGlassesP4Driver::InitSdspi(
//     const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
//   if (base_path == nullptr || base_path[0] == '\0' || max_freq_khz <= 0) {
//     return false;
//   }
//   if (!InitMinimal()) {
//     return false;
//   }
//   if (!DeinitSdmmc()) {
//     return false;
//   }
//
//   SdCard::SdspiConfig config;
//   config.host.slot = host_id;
//   config.host.max_freq_khz = max_freq_khz;
//   config.slot.host_id = host_id;
//   config.slot.gpio_cs = static_cast<gpio_num_t>(gpio::sd::kCs);
//   config.bus.mosi_io_num = gpio::sd::kMosi;
//   config.bus.miso_io_num = gpio::sd::kMiso;
//   config.bus.sclk_io_num = gpio::sd::kSclk;
//
//   const bool result = sd_card_.InitSdspi(base_path, config);
//   status_.sd_card.init_flag = sd_card_.IsMounted();
//   return result;
// }
//

// 以下为修改前 HEAD 的旧板实现，仅保留追溯，不可直接启用到当前新板。
// SY6970、ES8311、旧 I2C 分配和独立 5 V GPIO 均不匹配当前硬件。
// 旧摄像头电源逻辑会关闭共享 AVDD1；恢复应使用上方新板参考实现。
// void TGlassesP4Driver::CreateDrivers() {
//   if (platform_hal_ != nullptr) {
//     return;
//   }
//   platform_hal_ = std::make_unique<cpp_bus_driver::PlatformHal>();
//   screen_info_ = kDefaultScreenInfo;
//
//   bus_.sy6970_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
//       gpio::sy6970::kSda, gpio::sy6970::kScl, I2C_NUM_0);
//   bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c>(
//       gpio::sgm38121::kSda, gpio::sgm38121::kScl, I2C_NUM_1);
//
//   bus_.bq27220_i2c_bus =
//       std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sy6970_i2c_bus);
//   bus_.aw86224_i2c_bus =
//       std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
//   bus_.es8311_i2c_bus =
//       std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
//   bus_.screen_i2c_bus =
//       std::make_shared<cpp_bus_driver::HardwareI2c>(bus_.sgm38121_i2c_bus);
//
//   bus_.es8311_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
//       gpio::es8311::kAdcData, gpio::es8311::kDacData, gpio::es8311::kWsLrck,
//       gpio::es8311::kBclk, gpio::es8311::kMclk, i2s_port_t::I2S_NUM_0,
//       cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
//       cpp_bus_driver::HardwareI2s::I2sMode::kStd,
//       i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);
//
//   chip_.sy6970 = std::make_unique<cpp_bus_driver::Sy6970>(
//       bus_.sy6970_i2c_bus, device::sy6970::kI2cAddress);
//   chip_.bq27220 = std::make_unique<cpp_bus_driver::Bq27220>(
//       bus_.bq27220_i2c_bus, device::bq27220::kI2cAddress);
//   chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
//       bus_.sgm38121_i2c_bus, device::sgm38121::kI2cAddress);
//   chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
//       bus_.aw86224_i2c_bus, device::aw86224::kI2cAddress);
//   chip_.es8311 = std::make_unique<cpp_bus_driver::Es8311>(
//       bus_.es8311_i2c_bus, bus_.es8311_i2s_bus, device::es8311::kI2cAddress);
//   chip_.s023msafjf10111e1 =
//   std::make_unique<cpp_bus_driver::S023msafjf10111e1>(
//       bus_.screen_i2c_bus, device::s023msafjf10111e1::kI2cAddress,
//       gpio::s023msafjf10111e1::kRst);
// }
//
// bool TGlassesP4Driver::Init(InitMode mode) {
//   CreateDrivers();
//   const int64_t start_time_us = platform_hal_->GetSystemTimeUs();
//   const bool result = InitDrivers(mode);
//   const int64_t elapsed_time_us = platform_hal_->GetSystemTimeUs() -
//   start_time_us; LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
//       "TGlassesP4Driver init finished (mode: %s, result: %s, elapsed: "
//       "%lld ms)\n",
//       mode == InitMode::kAsync ? "async" : "sync",
//       result ? "success" : "failed",
//       static_cast<long long>(elapsed_time_us / 1000));
//   return result;
// }
//
// bool TGlassesP4Driver::InitMinimal() {
//   CreateDrivers();
//   return InitMinimalDrivers();
// }
//
// bool TGlassesP4Driver::InitMinimalDrivers() {
//   if (minimal_drivers_initialized_) {
//     return true;
//   }
//
//   bool result = true;
//   result &= InitSy6970();
//   result &= InitPower();
//   result &= InitSgm38121();
//   minimal_drivers_initialized_ = result;
//   return result;
// }
//
// bool TGlassesP4Driver::InitDrivers(InitMode mode) {
//   bool result = InitMinimalDrivers();
//   async_init_manager_.Reset();
//
//   if (mode == InitMode::kAsync) {
//     result &= async_init_manager_.StartTask(
//         [](void* arg) {
//           auto* self = static_cast<TGlassesP4Driver*>(arg);
//           if (!self->async_init_manager_.stop_requested()) {
//             self->InitScreen();
//           }
//           self->async_init_manager_.FinishTask();
//         },
//         "ScreenTask", 4096, this, 3);
//
//     result &= async_init_manager_.StartTask(
//         [](void* arg) {
//           auto* self = static_cast<TGlassesP4Driver*>(arg);
//           if (!self->async_init_manager_.stop_requested()) {
//             self->InitBq27220();
//           }
//           self->async_init_manager_.FinishTask();
//         },
//         "InitBq27220Task", 2048, this, 3);
//
//     result &= async_init_manager_.StartTask(
//         [](void* arg) {
//           auto* self = static_cast<TGlassesP4Driver*>(arg);
//           if (!self->async_init_manager_.stop_requested()) {
//             self->InitAw86224();
//           }
//           self->async_init_manager_.FinishTask();
//         },
//         "InitAw86224Task", 4096, this, 3);
//
//     result &= async_init_manager_.StartTask(
//         [](void* arg) {
//           auto* self = static_cast<TGlassesP4Driver*>(arg);
//           if (!self->async_init_manager_.stop_requested() &&
//               self->InitEs8311()) {
//             self->SetEs8311OperatingMode(Es8311OperatingMode::kSleep);
//           }
//           self->async_init_manager_.FinishTask();
//         },
//         "InitEs8311Task", 4096, this, 3);
//
//
//     result &= async_init_manager_.StartTask(
//         [](void* arg) {
//           auto* self = static_cast<TGlassesP4Driver*>(arg);
//           if (!self->async_init_manager_.stop_requested()) {
//             self->InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);
//           }
//           self->async_init_manager_.FinishTask();
//         },
//         "InitSdmmcTask", 4096, this, 3);
//   } else {
//     result &= InitScreen();
//     InitBq27220();
//     result &= InitAw86224();
//     bool es8311_initialized = InitEs8311();
//     if (es8311_initialized) {
//       es8311_initialized =
//       SetEs8311OperatingMode(Es8311OperatingMode::kSleep);
//     }
//     result &= es8311_initialized;
//
//     InitSdmmc(device::sd::kBasePath, SDMMC_FREQ_52M);
//
//     result &= status_.sy6970.init_flag;
//     result &= status_.sgm38121.init_flag;
//     result &= status_.bq27220.init_flag;
//     result &= status_.aw86224.init_flag;
//     result &= status_.es8311.init_flag;
//   }
//
//   return result;
// }
//
// bool TGlassesP4Driver::InitSy6970() {
//   if (!chip_.sy6970->Init()) {
//     status_.sy6970.init_flag = false;
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSy6970 failed\n");
//     return false;
//   }
//
//   status_.sy6970.init_flag = true;
//   LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSy6970 success\n");
//   return true;
// }
//
// 旧版电量计参数写入参考；恢复前需核对实际电池容量和 NTC 配置。
// bool TGlassesP4Driver::InitBq27220() {
//   if (!chip_.bq27220->Init()) {
//     status_.bq27220.init_flag = false;
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBq27220 failed\n");
//     return false;
//   }
//
//   cpp_bus_driver::Bq27220::CedvProfile battery_profile;
//   battery_profile.design_capacity = battery_info().capacity_mah;
//   battery_profile.full_charge_capacity = battery_info().capacity_mah;
//   cpp_bus_driver::Bq27220::GaugingConfig gauging_config;
//
//   bool result = true;
//   result &= chip_.bq27220->ApplyBatteryProfileIfNeeded(
//       battery_profile, gauging_config);
//   result &= chip_.bq27220->SetTemperatureMode(
//       cpp_bus_driver::Bq27220::TemperatureMode::kExternalNtc);
//
//   status_.bq27220.init_flag = result;
//   if (result) {
//     LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitBq27220 success\n");
//   } else {
//     chip_.bq27220->Deinit(false);
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitBq27220 failed\n");
//   }
//   return result;
// }
//
// bool TGlassesP4Driver::InitEs8311() {
//   if (IsEs8311Ready()) {
//     return true;
//   }
//
//   status_.es8311.init_flag = false;
//   if (chip_.es8311 == nullptr || !chip_.es8311->Init() ||
//       !chip_.es8311->Init(device::es8311::kMclkMultiple,
//           device::es8311::kSampleRate, device::es8311::kBitsPerSample)) {
//     if (chip_.es8311 != nullptr) {
//       chip_.es8311->Deinit(false);
//     }
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitEs8311 failed\n");
//     return false;
//   }
//
//   const cpp_bus_driver::Es8311::PowerStatus power_status = {
//       .contorl =
//           {
//               .analog_circuits = true,
//               .analog_bias_circuits = true,
//               .analog_adc_bias_circuits = true,
//               .analog_adc_reference_circuits = true,
//               .analog_dac_reference_circuit = true,
//               .internal_reference_circuits = false,
//           },
//       .vmid = cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge,
//   };
//   bool result = true;
//   result &= chip_.es8311->SetPowerStatus(power_status);
//   result &= chip_.es8311->SetPgaPower(true);
//   result &= chip_.es8311->SetAdcPower(true);
//   result &= chip_.es8311->SetDacPower(true);
//   result &= chip_.es8311->SetOutputToHpDrive(true);
//   result &= chip_.es8311->SetAdcOffsetFreeze(
//       cpp_bus_driver::Es8311::AdcOffsetFreeze::kDynamicHpf);
//   result &= chip_.es8311->SetAdcHpfStage2Coeff(10);
//   result &= chip_.es8311->SetDacEqualizer(false);
//   result &= chip_.es8311->SetMic(cpp_bus_driver::Es8311::MicType::kAnalogMic,
//       cpp_bus_driver::Es8311::MicInput::kMic1p1n);
//   result &= chip_.es8311->SetAdcAutoVolumeControl(false);
//   result &=
//       chip_.es8311->SetAdcGain(cpp_bus_driver::Es8311::AdcGain::kGain18Db);
//   result &= chip_.es8311->SetAdcPgaGain(
//       cpp_bus_driver::Es8311::AdcPgaGain::kGain30Db);
//   result &= chip_.es8311->SetAdcVolume(191);
//   result &= chip_.es8311->SetDacVolume(191);
//   if (!result) {
//     chip_.es8311->Deinit(false);
//   }
//   status_.es8311.init_flag = result;
//   LogMessage(result ? LogLevel::kInfo : LogLevel::kError, __FILE__, __LINE__,
//       result ? "InitEs8311 success\n" : "InitEs8311 failed\n");
//   return result;
// }
//
// bool TGlassesP4Driver::DeinitEs8311() {
//   bool result = true;
//   if (status_.es8311.init_flag && chip_.es8311 != nullptr) {
//     result &= SetEs8311OperatingMode(Es8311OperatingMode::kSleep);
//     result &= chip_.es8311->Deinit(false);
//   }
//   status_.es8311.init_flag = false;
//   return result;
// }
//
// bool TGlassesP4Driver::IsSy6970Ready() const {
//   return status_.sy6970.init_flag && chip_.sy6970 != nullptr;
// }
//
// bool TGlassesP4Driver::IsEs8311Ready() const {
//   return status_.es8311.init_flag && chip_.es8311 != nullptr;
// }
//
// bool TGlassesP4Driver::SetEs8311OperatingMode(Es8311OperatingMode mode) {
//   if (!status_.es8311.init_flag) {
//     return mode == Es8311OperatingMode::kSleep;
//   }
//   const bool playback_enabled = mode == Es8311OperatingMode::kPlayback ||
//                                 mode == Es8311OperatingMode::kDuplex;
//   const bool capture_enabled = mode == Es8311OperatingMode::kCapture ||
//                                mode == Es8311OperatingMode::kDuplex;
//   const bool sleep = mode == Es8311OperatingMode::kSleep;
//   cpp_bus_driver::Es8311::PowerStatus power_status = {
//       .contorl =
//           {
//               .analog_circuits = !sleep,
//               .analog_bias_circuits = !sleep,
//               .analog_adc_bias_circuits = capture_enabled,
//               .analog_adc_reference_circuits = capture_enabled,
//               .analog_dac_reference_circuit = playback_enabled,
//               .internal_reference_circuits = false,
//           },
//       .vmid = sleep
//                   ? cpp_bus_driver::Es8311::Vmid::kPowerDown
//                   :
//                   cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge,
//   };
//
//   bool result = true;
//   if (sleep) {
//     result &= chip_.es8311->SetOutputToHpDrive(false);
//     result &= chip_.es8311->SetPgaPower(false);
//     result &= chip_.es8311->SetAdcPower(false);
//     result &= chip_.es8311->SetDacPower(false);
//     result &= chip_.es8311->SetPowerStatus(power_status);
//   } else {
//     result &= chip_.es8311->SetPowerStatus(power_status);
//     result &= chip_.es8311->SetPgaPower(capture_enabled);
//     result &= chip_.es8311->SetAdcPower(capture_enabled);
//     result &= chip_.es8311->SetDacPower(playback_enabled);
//     result &= chip_.es8311->SetOutputToHpDrive(playback_enabled);
//   }
//   return result;
// }
//
// bool TGlassesP4Driver::InitPower() {
//   if (!InitLdoPower(4, 3300)) {
//     return false;
//   }
//   if (!InitLdoPower(3, 2500)) {
//     DeinitLdoPower(4);
//     return false;
//   }
//
//   bool gpio_initialized = true;
//   gpio_initialized &= platform_hal_->SetGpioMode(
//       gpio::power::kEn5v0, cpp_bus_driver::PlatformHal::GpioMode::kOutput);
//   gpio_initialized &= platform_hal_->SetGpioMode(
//       gpio::power::kEn3v3, cpp_bus_driver::PlatformHal::GpioMode::kOutput);
//   gpio_initialized &= platform_hal_->GpioWrite(gpio::power::kEn5v0, true);
//   gpio_initialized &= platform_hal_->GpioWrite(gpio::power::kEn3v3, true);
//   if (!gpio_initialized) {
//     platform_hal_->GpioWrite(gpio::power::kEn5v0, false);
//     platform_hal_->GpioWrite(gpio::power::kEn3v3, false);
//     DeinitLdoPower(3);
//     DeinitLdoPower(4);
//     return false;
//   }
//   platform_hal_->DelayMs(200);
//   return true;
// }
//
// bool TGlassesP4Driver::InitSgm38121() {
//   if (chip_.sgm38121 == nullptr || !chip_.sgm38121->Init()) {
//     status_.sgm38121.init_flag = false;
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121
//     failed\n"); return false;
//   }
//
//   bool result = true;
// #if defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_SC2336)
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd1,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd2,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
// #elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV2710)
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kDvdd1,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd1,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd2,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd2, 3000);
// #elif defined(CONFIG_LILYGO_DEVICE_DRIVER_CAMERA_TYPE_OV5645)
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kDvdd1,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd1,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetChannelStatus(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd2,
//       cpp_bus_driver::Sgm38121::Status::kOff);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
//   result &= chip_.sgm38121->SetOutputVoltage(
//       cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
// #endif
//
//   status_.sgm38121.init_flag = result;
//   if (!result) {
//     chip_.sgm38121->Deinit(false);
//   }
//   if (result) {
//     LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSgm38121
//     success\n");
//   } else {
//     LogMessage(LogLevel::kError, __FILE__, __LINE__, "InitSgm38121
//     failed\n");
//   }
//   return result;
// }
//
// bool TGlassesP4Driver::PrepareDriversForPowerOff() {
//   if (!async_init_manager_.StopAndWait(kInitializationShutdownTimeoutMs)) {
//     LogMessage(LogLevel::kWarning, __FILE__, __LINE__,
//         "Wait for asynchronous initialization before power off timed out\n");
//     return false;
//   }
//
//   bool result = true;
//   result &= DeinitScreen();
//   result &= DeinitAw86224();
//   result &= DeinitEs8311();
//   result &= SetCameraPowerEnabled(false);
//   if (IsSdmmcReady()) {
//     result &= DeinitSdmmc();
//   }
//   result &= platform_hal_->GpioWrite(gpio::power::kEn5v0, 0);
//   result &= platform_hal_->GpioWrite(gpio::power::kEn3v3, 0);
//   minimal_drivers_initialized_ = false;
//   return result;
// }

}  // namespace lilygo_device_driver
