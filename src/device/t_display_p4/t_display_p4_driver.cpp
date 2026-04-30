/*
 * @Description: None
 * @Author: LILYGO_L
 * @Date: 2026-01-22 13:51:14
 * @LastEditTime: 2026-04-30 14:44:32
 * @License: GPL 3.0
 */
#include "t_display_p4_driver.h"

namespace lilygo_device_driver {

TDisplayP4Driver& TDisplayP4Driver::GetInstance() {
  static TDisplayP4Driver* instance = new TDisplayP4Driver();
  return *instance;
}

void TDisplayP4Driver::CreateDrivers() {
  tool_ = std::make_unique<cpp_bus_driver::Tool>();

  bus_.bq27220_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      BQ27220_SDA, BQ27220_SCL, I2C_NUM_0);
  bus_.xl9535_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      XL9535_SDA, XL9535_SCL, I2C_NUM_0);
  bus_.sgm38121_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      SGM38121_SDA, SGM38121_SCL, I2C_NUM_1);
  bus_.pcf8563_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      PCF8563_SDA, PCF8563_SCL, I2C_NUM_0);
  bus_.aw86224_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      AW86224_SDA, AW86224_SCL, I2C_NUM_1);
  bus_.es8311_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      ES8311_SDA, ES8311_SCL, I2C_NUM_1);

  bus_.screen_mipi_bus = std::make_shared<cpp_bus_driver::HardwareMipi>(
      SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_MIPI_DSI_HSYNC, SCREEN_MIPI_DSI_HBP,
      SCREEN_MIPI_DSI_HFP, SCREEN_MIPI_DSI_VSYNC, SCREEN_MIPI_DSI_VBP,
      SCREEN_MIPI_DSI_VFP, SCREEN_DATA_LANE_NUM,
      [](uint8_t format) -> cpp_bus_driver::HardwareMipi::ColorFormat {
        switch (format) {
          case 16:
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
          case 24:
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb888;
          default:
            return cpp_bus_driver::HardwareMipi::ColorFormat::kRgb565;
        }
      }(SCREEN_BITS_PER_PIXEL));

  bus_.es8311_i2s_bus = std::make_shared<cpp_bus_driver::HardwareI2s>(
      ES8311_ADC_DATA, ES8311_DAC_DATA, ES8311_WS_LRCK, ES8311_BCLK,
      ES8311_MCLK, i2s_port_t::I2S_NUM_0,
      cpp_bus_driver::HardwareI2s::DataMode::kInputOutput,
      cpp_bus_driver::HardwareI2s::I2sMode::kStd,
      i2s_clock_src_t::I2S_CLK_SRC_DEFAULT);

  bus_.l76k_uart_bus = std::make_shared<cpp_bus_driver::HardwareUart>(
      GPS_RX, GPS_TX, UART_NUM_1);

  bus_.icm20948_i2c_bus = std::make_unique<TwoWire>(1);

  bus_.sx1262_spi_bus = std::make_shared<cpp_bus_driver::HardwareSpi>(
      SX1262_MOSI, SX1262_SCLK, SX1262_MISO, SPI2_HOST, 0);

#if defined CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0
  bus_.bq25896_i2c_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      BQ25896_SDA, BQ25896_SCL, I2C_NUM_0);

  chip_.bq25896_dev = std::make_shared<kode_bq25896::bq25896_dev_t>();
  chip_.bq25896_handle = chip_.bq25896_dev.get();
#endif

  chip_.bq27220 = std::make_unique<cpp_bus_driver::Bq27220xxxx>(
      bus_.bq27220_i2c_bus, BQ27220_I2C_ADDRESS);

  chip_.xl9535 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9535_i2c_bus, XL9535_I2C_ADDRESS);

  chip_.sgm38121 = std::make_unique<cpp_bus_driver::Sgm38121>(
      bus_.sgm38121_i2c_bus, SGM38121_I2C_ADDRESS);

#if defined CONFIG_SCREEN_TYPE_HI8561
  bus_.hi8561_i2c_touch_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      HI8561_TOUCH_SDA, HI8561_TOUCH_SCL, I2C_NUM_0);

  chip_.hi8561 = std::make_unique<cpp_bus_driver::Hi8561>(bus_.screen_mipi_bus);
  chip_.hi8561_touch = std::make_unique<cpp_bus_driver::Hi8561Touch>(
      bus_.hi8561_i2c_touch_bus, HI8561_TOUCH_I2C_ADDRESS);
  chip_.hi8561_backlight =
      std::make_unique<cpp_bus_driver::Pwm>(HI8561_SCREEN_BL);
#elif defined CONFIG_SCREEN_TYPE_RM69A10
  bus_.gt9895_i2c_touch_bus = std::make_shared<cpp_bus_driver::HardwareI2c1>(
      GT9895_SDA, GT9895_SCL, I2C_NUM_0);

  chip_.rm69a10 =
      std::make_unique<cpp_bus_driver::Rm69a10>(bus_.screen_mipi_bus);
  chip_.gt9895 =
      std::make_unique<cpp_bus_driver::Gt9895>(bus_.gt9895_i2c_touch_bus,
          GT9895_I2C_ADDRESS, -1, GT9895_X_SCALE_FACTOR, GT9895_Y_SCALE_FACTOR);
#endif

  chip_.pcf8563 = std::make_unique<cpp_bus_driver::Pcf8563x>(
      bus_.pcf8563_i2c_bus, PCF8563_I2C_ADDRESS);

  chip_.aw86224 = std::make_unique<cpp_bus_driver::Aw862xx>(
      bus_.aw86224_i2c_bus, AW86224_I2C_ADDRESS);

  chip_.es8311 = std::make_unique<cpp_bus_driver::Es8311>(
      bus_.es8311_i2c_bus, bus_.es8311_i2s_bus, ES8311_I2C_ADDRESS);

  chip_.icm20948 = std::make_unique<ICM20948_WE>(
      bus_.icm20948_i2c_bus.get(), ICM20948_I2C_ADDRESS);

  chip_.l76k = std::make_unique<cpp_bus_driver::L76k>(
      bus_.l76k_uart_bus, [this](bool value) -> bool {
        return chip_.xl9535->GpioWrite(XL9535_GPS_WAKE_UP,
            static_cast<cpp_bus_driver::Xl95x5::Value>(value));
      });

  chip_.sx1262 = std::make_unique<cpp_bus_driver::Sx126x>(bus_.sx1262_spi_bus,
      cpp_bus_driver::Sx126x::ChipType::kSx1262, SX1262_BUSY, SX1262_CS);

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
  bus_.xl9555_i2c_bus =
      std::make_shared<cpp_bus_driver::SoftwareI2c>(XL9555_SDA, XL9555_SCL);
  bus_.tca8418_i2c_bus =
      std::make_shared<cpp_bus_driver::SoftwareI2c>(TCA8418_SDA, TCA8418_SCL);

  bus_.cc1101_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(T_MIXRF_CC1101_MOSI,
          T_MIXRF_CC1101_SCLK, T_MIXRF_CC1101_MISO, SPI2_HOST, 0);
  bus_.nrf24l01_spi_bus =
      std::make_shared<cpp_bus_driver::HardwareSpi>(T_MIXRF_NRF24L01_MOSI,
          T_MIXRF_NRF24L01_SCLK, T_MIXRF_NRF24L01_MISO, SPI2_HOST, 0);

  bus_.cc1101_radiolib_hal = new RadiolibCppBusDriverHal(
      bus_.cc1101_spi_bus, 10000000, T_MIXRF_CC1101_CS);
  bus_.nrf24l01_radiolib_hal = new RadiolibCppBusDriverHal(
      bus_.nrf24l01_spi_bus, 10000000, T_MIXRF_NRF24L01_CS);

  bus_.cc1101_module = new Module(bus_.cc1101_radiolib_hal,
      static_cast<uint32_t>(RADIOLIB_NC), static_cast<uint32_t>(RADIOLIB_NC),
      static_cast<uint32_t>(RADIOLIB_NC), T_MIXRF_CC1101_BUSY);

  bus_.nrf24l01_module =
      new Module(bus_.nrf24l01_radiolib_hal, static_cast<uint32_t>(RADIOLIB_NC),
          static_cast<uint32_t>(T_MIXRF_NRF24L01_INT),
          static_cast<uint32_t>(T_MIXRF_NRF24L01_CE),
          static_cast<uint32_t>(RADIOLIB_NC));

  chip_.xl9555 = std::make_unique<cpp_bus_driver::Xl95x5>(
      bus_.xl9555_i2c_bus, XL9555_I2C_ADDRESS);
  chip_.tca8418 = std::make_unique<cpp_bus_driver::Tca8418>(
      bus_.tca8418_i2c_bus, TCA8418_I2C_ADDRESS);
  chip_.tca8418_backlight = std::make_unique<cpp_bus_driver::Pwm>(TCA8418_BL);

  chip_.cc1101 = new CC1101(bus_.cc1101_module);
  chip_.nrf24l01 = new nRF24(bus_.nrf24l01_module);
#endif
}

bool TDisplayP4Driver::InitDrivers(InitMode mode) {
  bool result = true;

  result &= InitEsp32p4();

#if defined CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0
  InitBq25896();
  result &=
      bus_.xl9535_i2c_bus->set_bus_handle(bus_.bq25896_i2c_bus->bus_handle());
#endif

  InitXl9535();
  InitPower();
  result &= ConfigXl9535();

  InitSgm38121();

#if defined CONFIG_SCREEN_TYPE_HI8561
  result &= bus_.hi8561_i2c_touch_bus->set_bus_handle(
      bus_.xl9535_i2c_bus->bus_handle());
#elif defined CONFIG_SCREEN_TYPE_RM69A10
  result &= bus_.gt9895_i2c_touch_bus->set_bus_handle(
      bus_.xl9535_i2c_bus->bus_handle());
#endif

  result &=
      bus_.bq27220_i2c_bus->set_bus_handle(bus_.xl9535_i2c_bus->bus_handle());
  result &=
      bus_.pcf8563_i2c_bus->set_bus_handle(bus_.xl9535_i2c_bus->bus_handle());
  result &=
      bus_.aw86224_i2c_bus->set_bus_handle(bus_.sgm38121_i2c_bus->bus_handle());
  result &=
      bus_.es8311_i2c_bus->set_bus_handle(bus_.sgm38121_i2c_bus->bus_handle());
#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
  bus_.cc1101_spi_bus->set_bus_init_flag(true);
  bus_.nrf24l01_spi_bus->set_bus_init_flag(true);
#endif

  result &= bus_.icm20948_i2c_bus->set_bus_handle(
      bus_.sgm38121_i2c_bus->bus_handle());
  result &= bus_.icm20948_i2c_bus->begin(ICM20948_SDA, ICM20948_SCL);

  if (mode == InitMode::kAsync) {
    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
#if defined CONFIG_SCREEN_TYPE_HI8561
          self->InitHi8561();
#elif defined CONFIG_SCREEN_TYPE_RM69A10
          self->InitRm69a10();
#endif
          vTaskDelete(NULL);
        },
        "ScreenTask", 4096, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
#if defined CONFIG_SCREEN_TYPE_HI8561
          self->InitHi8561Touch();
          self->InitHi8561Backlight();
#elif defined CONFIG_SCREEN_TYPE_RM69A10
          self->InitGt9895();
#endif
          vTaskDelete(NULL);
        },
        "TouchAndBacklightTask", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitBq27220();
          vTaskDelete(NULL);
        },
        "InitBq27220Task", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitPcf8563();
          vTaskDelete(NULL);
        },
        "InitPcf8563Task", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitAw86224();
          vTaskDelete(NULL);
        },
        "InitAw86224Task", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitEs8311();
          self->ConfigEs8311();
          vTaskDelete(NULL);
        },
        "InitConfigEs8311Task", 4096, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitL76k();
          vTaskDelete(NULL);
        },
        "InitL76kTask", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitIcm20948();
          vTaskDelete(NULL);
        },
        "InitIcm20948Task", 4096, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitSx1262();
          vTaskDelete(NULL);
        },
        "InitSx1262Task", 2048, this, 3, NULL);

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitXl9555();
          self->ConfigXl9555();
          vTaskDelete(NULL);
        },
        "InitConfigXl9555Task", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitTca8418();
          self->InitTca8418Backlight();
          vTaskDelete(NULL);
        },
        "InitTca8418AndBacklightTask", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitCc1101();
          vTaskDelete(NULL);
        },
        "InitCc1101Task", 2048, this, 3, NULL);

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitNrf24l01();
          vTaskDelete(NULL);
        },
        "InitNrf24l01Task", 2048, this, 3, NULL);
#endif

    xTaskCreate(
        [](void* arg) {
          auto self = static_cast<TDisplayP4Driver*>(arg);
          self->InitSdmmc(SD_BASE_PATH, SDMMC_FREQ_52M);
          vTaskDelete(NULL);
        },
        "InitSdmmcTask", 4096, this, 3, NULL);

    result = true;
  } else {
#if defined CONFIG_SCREEN_TYPE_HI8561
    InitHi8561();
    InitHi8561Touch();
    InitHi8561Backlight();
#elif defined CONFIG_SCREEN_TYPE_RM69A10
    InitRm69a10();
    InitGt9895();
#endif

    InitBq27220();
    InitPcf8563();
    InitAw86224();
    InitEs8311();
    result &= ConfigEs8311();
    InitL76k();
    InitIcm20948();
    InitSx1262();

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
    InitXl9555();
    result &= ConfigXl9555();
    InitTca8418();
    InitTca8418Backlight();
    InitCc1101();
    InitNrf24l01();
#endif

    InitSdmmc(SD_BASE_PATH, SDMMC_FREQ_52M);

#if defined CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0
    result &= status_.bq25896.init_flag;
#endif

    result &= status_.xl9535.init_flag;
    result &= status_.sgm38121.init_flag;

#if defined CONFIG_SCREEN_TYPE_HI8561
    result &= status_.hi8561.init_flag;
    result &= status_.hi8561_touch.init_flag;
#elif defined CONFIG_SCREEN_TYPE_RM69A10
    result &= status_.rm69a10.init_flag;
    result &= status_.gt9895.init_flag;
#endif

    result &= status_.bq27220.init_flag;
    result &= status_.pcf8563.init_flag;
    result &= status_.aw86224.init_flag;
    result &= status_.es8311.init_flag;
    result &= status_.l76k.init_flag;
    result &= status_.icm20948.init_flag;
    result &= status_.sx1262.init_flag;

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
    result &= status_.xl9555.init_flag;
    result &= status_.tca8418.init_flag;
    result &= status_.cc1101.init_flag;
    result &= status_.nrf24l01.init_flag;
#endif

    result &= status_.sd_card.init_flag;
  }

  return result;
}

bool TDisplayP4Driver::Init(InitMode mode) {
  CreateDrivers();
  return InitDrivers(mode);
}

bool TDisplayP4Driver::SetSleep(SleepLevel level, bool enable) {
  bool result = true;

  switch (level) {
    case SleepLevel::kChipSleep:
      if (enable) {
#if defined CONFIG_SCREEN_TYPE_HI8561
        if (status_.hi8561_backlight.init_flag) {
          result &= chip_.hi8561_backlight->Stop(0);
        }
        if (status_.hi8561.init_flag) {
          result &= chip_.hi8561->SetScreenOff(true);
          result &= chip_.hi8561->SetSleep(true);
        }
#elif defined CONFIG_SCREEN_TYPE_RM69A10
        if (status_.rm69a10.init_flag) {
          result &= chip_.rm69a10->SetBrightness(0);
          result &= chip_.rm69a10->SetScreenOff(true);
          result &= chip_.rm69a10->SetSleep(true);
        }
        if (status_.gt9895.init_flag) {
          result &= chip_.gt9895->SetSleep();
        }
#endif

        if (status_.es8311.init_flag) {
          cpp_bus_driver::Es8311::PowerStatus ps = {
              .contorl =
                  {
                      .analog_circuits = false,
                      .analog_bias_circuits = false,
                      .analog_adc_bias_circuits = false,
                      .analog_adc_reference_circuits = false,
                      .analog_dac_reference_circuit = false,
                      .internal_reference_circuits = false,
                  },
              .vmid = cpp_bus_driver::Es8311::Vmid::kPowerDown,
          };
          result &= chip_.es8311->SetOutputToHpDrive(false);
          result &= chip_.es8311->SetPgaPower(false);
          result &= chip_.es8311->SetAdcPower(false);
          result &= chip_.es8311->SetDacPower(false);
          result &= chip_.es8311->SetPowerStatus(ps);
        }

        if (status_.icm20948.init_flag) {
          chip_.icm20948->sleep(true);
        }

        if (status_.l76k.init_flag) {
          result &= chip_.l76k->Sleep(true);
        }

        if (status_.sx1262.init_flag) {
          result &= chip_.sx1262->SetStandby(
              cpp_bus_driver::Sx126x::StdbyConfig::kStdbyRc);
          result &= chip_.sx1262->SetSleep(
              cpp_bus_driver::Sx126x::SleepMode::kWarmStart);
        }

        if (status_.sgm38121.init_flag) {
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd1,
              cpp_bus_driver::Sgm38121::Status::kOff);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd2,
              cpp_bus_driver::Sgm38121::Status::kOff);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd1,
              cpp_bus_driver::Sgm38121::Status::kOff);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd2,
              cpp_bus_driver::Sgm38121::Status::kOff);
        }

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
        if (status_.cc1101.init_flag) {
          int16_t ret = chip_.cc1101->sleep();
          if (ret != RADIOLIB_ERR_NONE) {
            LogMessage(LogLevel::kChip, __FILE__, __LINE__,
                "cc1101 sleep failed (error code: %d)\n", ret);
            result = false;
          }
        }

        if (status_.nrf24l01.init_flag) {
          int16_t ret = chip_.nrf24l01->sleep();
          if (ret != RADIOLIB_ERR_NONE) {
            LogMessage(LogLevel::kChip, __FILE__, __LINE__,
                "nrf24l01 sleep failed (error code: %d)\n", ret);
            result = false;
          }
        }

        if (status_.tca8418_backlight.init_flag) {
          result &= chip_.tca8418_backlight->Stop(0);
        }

#endif

        if (status_.xl9535.init_flag) {
          result &= chip_.xl9535->GpioWrite(
              XL9535_ESP32C6_EN, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              XL9535_SD_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9535->GpioWrite(
              XL9535_5_0_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kLow);
        }
      } else {
        if (status_.xl9535.init_flag) {
          result &= chip_.xl9535->GpioWrite(
              XL9535_ESP32C6_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9535->GpioWrite(
              XL9535_SD_EN, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              XL9535_5_0_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
        }

#if defined CONFIG_SCREEN_TYPE_HI8561
        if (status_.hi8561.init_flag) {
          result &= chip_.hi8561->SetSleep(false);
          result &= chip_.hi8561->SetScreenOff(false);
        }
#elif defined CONFIG_SCREEN_TYPE_RM69A10
        if (status_.rm69a10.init_flag) {
          result &= chip_.rm69a10->SetSleep(false);
          result &= chip_.rm69a10->SetScreenOff(false);
        }
        if (status_.gt9895.init_flag) {
          result &= InitGt9895();
        }
#endif

        if (status_.es8311.init_flag) {
          if (status_.es8311.init_flag) {
            cpp_bus_driver::Es8311::PowerStatus ps = {
                .contorl =
                    {
                        .analog_circuits = true,
                        .analog_bias_circuits = true,
                        .analog_adc_bias_circuits = true,
                        .analog_adc_reference_circuits = true,
                        .analog_dac_reference_circuit = true,
                        .internal_reference_circuits = false,
                    },
                .vmid =
                    cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge,
            };
            result &= chip_.es8311->SetOutputToHpDrive(true);
            result &= chip_.es8311->SetPgaPower(true);
            result &= chip_.es8311->SetAdcPower(true);
            result &= chip_.es8311->SetDacPower(true);
            result &= chip_.es8311->SetPowerStatus(ps);
          }
        }

        if (status_.icm20948.init_flag) {
          chip_.icm20948->sleep(false);
        }

        if (status_.l76k.init_flag) {
          result &= chip_.l76k->Sleep(false);
        }

        if (status_.sx1262.init_flag) {
          // 唤醒
          tool_->GpioWrite(SX1262_CS, 1);
          tool_->DelayMs(10);
          tool_->GpioWrite(SX1262_CS, 0);
          tool_->DelayMs(10);
          tool_->GpioWrite(SX1262_CS, 1);
          tool_->DelayMs(10);
        }

        if (status_.sgm38121.init_flag) {
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd1,
              cpp_bus_driver::Sgm38121::Status::kOn);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kDvdd2,
              cpp_bus_driver::Sgm38121::Status::kOn);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd1,
              cpp_bus_driver::Sgm38121::Status::kOn);
          result &= chip_.sgm38121->SetChannelStatus(
              cpp_bus_driver::Sgm38121::Channel::kAvdd2,
              cpp_bus_driver::Sgm38121::Status::kOn);
        }
      }
      break;
    case SleepLevel::kPowerOff:
      if (enable) {
        if (status_.icm20948.init_flag) {
          chip_.icm20948->sleep(true);
        }

        if (status_.l76k.init_flag) {
          result &= chip_.l76k->Sleep(true);
        }

        if (status_.sx1262.init_flag) {
          result &= chip_.sx1262->SetStandby(
              cpp_bus_driver::Sx126x::StdbyConfig::kStdbyRc);
          result &= chip_.sx1262->SetSleep(
              cpp_bus_driver::Sx126x::SleepMode::kWarmStart);
        }

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
        if (status_.xl9555.init_flag) {
          result &= chip_.xl9555->GpioWrite(
              XL9555_LED_1, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(
              XL9555_LED_2, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(
              XL9555_LED_3, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9555->GpioWrite(
              XL9555_T_MIXRF_EN, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9555->GpioWrite(
              XL9555_TCA8418_RST, cpp_bus_driver::Xl95x5::Value::kLow);
        }
#endif
        if (status_.xl9535.init_flag) {
          result &= chip_.xl9535->GpioWrite(
              XL9535_SCREEN_RST, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              XL9535_TOUCH_RST, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              XL9535_ESP32C6_EN, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              XL9535_ETHERNET_RST, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              XL9535_SD_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
          result &= chip_.xl9535->GpioWrite(
              XL9535_5_0_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kLow);
          result &= chip_.xl9535->GpioWrite(
              XL9535_3_3_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kHigh);

          result &= chip_.aw86224->Deinit();
          status_.aw86224.init_flag = false;
          result &= chip_.es8311->Deinit();
          status_.es8311.init_flag = false;
          result &= bus_.icm20948_i2c_bus->end(false);
          status_.icm20948.init_flag = false;

#if defined CONFIG_SCREEN_TYPE_HI8561
          result &= chip_.hi8561_touch->Deinit();
          status_.hi8561_touch.init_flag = false;
          result &= chip_.hi8561_backlight->Stop(0);
          status_.hi8561_backlight.init_flag = false;
#elif defined CONFIG_SCREEN_TYPE_RM69A10
          result &= chip_.gt9895->Deinit();
          status_.gt9895.init_flag = false;
#endif

          result &= chip_.bq27220->Deinit();
          status_.bq27220.init_flag = false;
          result &= chip_.pcf8563->Deinit();
          status_.pcf8563.init_flag = false;

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
          result &= chip_.xl9555->Deinit();
          status_.xl9555.init_flag = false;
          result &= chip_.tca8418->Deinit();
          status_.tca8418.init_flag = false;
          result &= chip_.tca8418_backlight->Stop(0);
          status_.tca8418_backlight.init_flag = false;
#endif

#if defined CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0
          result &= bus_.bq25896_i2c_bus->Deinit();
          status_.bq25896.init_flag = false;
#endif

          result &= chip_.sgm38121->Deinit(true);
          status_.sgm38121.init_flag = false;
          result &= chip_.xl9535->Deinit(true);
          status_.xl9535.init_flag = false;

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
          result &= bus_.cc1101_spi_bus->Deinit();
          status_.cc1101.init_flag = false;
          result &= bus_.nrf24l01_spi_bus->Deinit();
          status_.nrf24l01.init_flag = false;
#endif

          result &= chip_.sx1262->Deinit(true);
          status_.sx1262.init_flag = false;
          result &= chip_.l76k->Deinit();
          status_.l76k.init_flag = false;

#if defined CONFIG_SCREEN_TYPE_HI8561
          result &= chip_.hi8561->Deinit();
          status_.hi8561.init_flag = false;
#elif defined CONFIG_SCREEN_TYPE_RM69A10
          result &= chip_.rm69a10->Deinit();
          status_.rm69a10.init_flag = false;
#endif
        }
      } else {
        result &= InitDrivers(InitMode::kAsync);
      }
      break;

    default:
      break;
  }

  return result;
}

bool TDisplayP4Driver::InitEsp32p4() {
  bool result = true;

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
  tool_->SetGpioMode(
      T_MIXRF_CC1101_CS, cpp_bus_driver::Tool::GpioMode::kOutput);
  tool_->SetGpioMode(
      T_MIXRF_NRF24L01_CS, cpp_bus_driver::Tool::GpioMode::kOutput);
  tool_->SetGpioMode(
      T_MIXRF_ST25R3916_CS, cpp_bus_driver::Tool::GpioMode::kOutput);
  tool_->GpioWrite(T_MIXRF_CC1101_CS, 1);
  tool_->GpioWrite(T_MIXRF_NRF24L01_CS, 1);
  tool_->GpioWrite(T_MIXRF_ST25R3916_CS, 1);

  tool_->SetGpioMode(T_MIXRF_CC1101_BUSY,
      cpp_bus_driver::Tool::GpioMode::kInput,
      cpp_bus_driver::Tool::GpioStatus::kPulldown);
#endif

  return result;
}

bool TDisplayP4Driver::InitPower() {
  bool result = true;
  result &= InitLdoPower(3, 2500);
  result &= InitLdoPower(4, 3300);
  return result;
}

#if defined CONFIG_BOARD_VERSION_T_DISPLAY_P4_V2_0
bool TDisplayP4Driver::InitBq25896() {
  int16_t ret =
      kode_bq25896::bq25896_init(bus_.bq25896_i2c_bus, chip_.bq25896_handle);
  if (ret != ESP_OK) {
    status_.bq25896.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "InitBq25896 failed (error code: %#X)\n", ret);
    return false;
  } else {
    bool result = true;

    ret = kode_bq25896::bq25896_set_input_current_limit(chip_.bq25896_handle,
        kode_bq25896::bq25896_ilim_t::BQ25896_ILIM_2000MA);
    if (ret != ESP_OK) {
      result = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "bq25896_set_input_current_limit failed (error code: %#X)\n", ret);
    }
    // 禁用看门狗后不能读取看门狗寄存器状态，否则看门狗禁用会失效
    ret = kode_bq25896::bq25896_set_watchdog_timer(chip_.bq25896_handle,
        kode_bq25896::bq25896_watchdog_t::BQ25896_WATCHDOG_DISABLE);
    if (ret != ESP_OK) {
      result = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "bq25896_set_watchdog_timer failed (error code: %#X)\n", ret);
    }
    ret = kode_bq25896::bq25896_set_charge_current(
        chip_.bq25896_handle, kode_bq25896::bq25896_ichg_t::BQ25896_ICHG_512MA);
    if (ret != ESP_OK) {
      result = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "bq25896_set_charge_current failed (error code: %#X)\n", ret);
    }

    status_.bq25896.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitBq25896 success\n");
    }
    return result;
  }
}
#endif

bool TDisplayP4Driver::InitBq27220() {
  if (!chip_.bq27220->Init()) {
    status_.bq27220.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitBq27220 failed\n");
    return false;
  } else {
    // 设置的电池容量会在没有电池插入的时候自动还原为默认值
    bool result = true;
    result &= chip_.bq27220->SetDesignCapacity(1000);
    result &= chip_.bq27220->SetTemperatureMode(
        cpp_bus_driver::Bq27220xxxx::TemperatureMode::kExternalNtc);

    status_.bq27220.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitBq27220 success\n");
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitBq27220 failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitXl9535() {
  if (!chip_.xl9535->Init()) {
    status_.xl9535.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitXl9535 failed\n");
    return false;
  } else {
    status_.xl9535.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9535 success\n");
    return true;
  }
}

bool TDisplayP4Driver::ConfigXl9535() {
  if (!status_.xl9535.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9535 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.xl9535->SetGpioMode(
      XL9535_SCREEN_RST, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_TOUCH_RST, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_ESP32P4_VCCA_POWER_EN, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_5_0_V_POWER_EN, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_3_3_V_POWER_EN, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_GPS_WAKE_UP, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_ESP32C6_EN, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_ETHERNET_RST, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_SD_EN, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_SX1262_RST, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_SKY13453_VCTL, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_ICM20948_INT, cpp_bus_driver::Xl95x5::Mode::kInput);
  result &= chip_.xl9535->SetGpioMode(
      XL9535_SX1262_DIO1, cpp_bus_driver::Xl95x5::Mode::kInput);

  result &= chip_.xl9535->GpioWrite(
      XL9535_ESP32P4_VCCA_POWER_EN, cpp_bus_driver::Xl95x5::Value::kLow);
  // 默认使用RF1天线
  result &= chip_.xl9535->GpioWrite(
      XL9535_SKY13453_VCTL, cpp_bus_driver::Xl95x5::Value::kHigh);

  result &= chip_.xl9535->GpioWrite(
      XL9535_3_3_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(100);
  result &= chip_.xl9535->GpioWrite(
      XL9535_3_3_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(100);
  result &= chip_.xl9535->GpioWrite(
      XL9535_3_3_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(200);

  result &= chip_.xl9535->GpioWrite(
      XL9535_SCREEN_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_TOUCH_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_ESP32C6_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_ETHERNET_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_GPS_WAKE_UP, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SX1262_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SD_EN, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_5_0_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SCREEN_RST, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_TOUCH_RST, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_ESP32C6_EN, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_ETHERNET_RST, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_GPS_WAKE_UP, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SX1262_RST, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SD_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_5_0_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(10);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SCREEN_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_TOUCH_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_ESP32C6_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_ETHERNET_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_GPS_WAKE_UP, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SX1262_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9535->GpioWrite(
      XL9535_SD_EN, cpp_bus_driver::Xl95x5::Value::kLow);
  result &= chip_.xl9535->GpioWrite(
      XL9535_5_0_V_POWER_EN, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(120);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9535 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitSgm38121() {
  if (!chip_.sgm38121->Init()) {
    status_.sgm38121.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitSgm38121 failed\n");
    return false;
  } else {
    bool result = true;
#if defined CONFIG_CAMERA_TYPE_SC2336
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1,
        cpp_bus_driver::Sgm38121::Status::kOn);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd2,
        cpp_bus_driver::Sgm38121::Status::kOn);
#elif defined CONFIG_CAMERA_TYPE_OV2710
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kAvdd2, 3000);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kDvdd1,
        cpp_bus_driver::Sgm38121::Status::kOn);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1,
        cpp_bus_driver::Sgm38121::Status::kOn);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd2,
        cpp_bus_driver::Sgm38121::Status::kOn);
#elif defined CONFIG_CAMERA_TYPE_OV5645
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kDvdd1, 1500);
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1, 1800);
    result &= chip_.sgm38121->SetOutputVoltage(
        cpp_bus_driver::Sgm38121::Channel::kAvdd2, 2800);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kDvdd1,
        cpp_bus_driver::Sgm38121::Status::kOn);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd1,
        cpp_bus_driver::Sgm38121::Status::kOn);
    result &= chip_.sgm38121->SetChannelStatus(
        cpp_bus_driver::Sgm38121::Channel::kAvdd2,
        cpp_bus_driver::Sgm38121::Status::kOn);
#endif

    status_.sgm38121.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSgm38121 success\n");
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitSgm38121 failed\n");
    }
    return result;
  }
}

#if defined CONFIG_SCREEN_TYPE_HI8561
bool TDisplayP4Driver::InitHi8561() {
  if (!chip_.hi8561->Init(
          SCREEN_MIPI_DSI_DPI_CLK_MHZ, SCREEN_LANE_BIT_RATE_MBPS)) {
    status_.hi8561.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitHi8561 failed\n");
    return false;
  } else {
    status_.hi8561.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitHi8561Touch() {
  if (!chip_.hi8561_touch->Init()) {
    status_.hi8561_touch.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Touch failed\n");
    return false;
  } else {
    status_.hi8561_touch.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Touch success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitHi8561Backlight() {
  if (!chip_.hi8561_backlight->Init(
          ledc_timer_t::LEDC_TIMER_0, ledc_channel_t::LEDC_CHANNEL_0, 2000)) {
    status_.hi8561_backlight.init_flag = false;
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "InitHi8561Backlight failed\n");
    return false;
  } else {
    status_.hi8561_backlight.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitHi8561Backlight success\n");
    return true;
  }
}
#elif defined CONFIG_SCREEN_TYPE_RM69A10
bool TDisplayP4Driver::InitRm69a10() {
  if (!chip_.rm69a10->Init(
          SCREEN_MIPI_DSI_DPI_CLK_MHZ, SCREEN_LANE_BIT_RATE_MBPS)) {
    status_.rm69a10.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitRm69a10 failed\n");
    return false;
  } else {
    status_.rm69a10.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitRm69a10 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitGt9895() {
  if (!chip_.gt9895->Init()) {
    status_.gt9895.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitGt9895 failed\n");
    return false;
  } else {
    status_.gt9895.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitGt9895 success\n");
    return true;
  }
}
#endif

bool TDisplayP4Driver::InitPcf8563() {
  if (!chip_.pcf8563->Init()) {
    status_.pcf8563.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitPcf8563 failed\n");
    return false;
  } else {
    status_.pcf8563.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitPcf8563 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitAw86224() {
  if (!chip_.aw86224->Init(500000)) {
    status_.aw86224.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitAw86224 failed\n");
    return false;
  } else {
    status_.aw86224.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitAw86224 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitEs8311() {
  if (!chip_.es8311->Init()) {
    status_.es8311.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8311 failed\n");
    return false;
  } else {
    if (!chip_.es8311->Init(
            ES8311_MCLK_MULTIPLE, ES8311_SAMPLE_RATE, ES8311_BITS_PER_SAMPLE)) {
      status_.es8311.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitEs8311 failed\n");
      return false;
    } else {
      status_.es8311.init_flag = true;
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitEs8311 success\n");
      return true;
    }
  }
}

bool TDisplayP4Driver::ConfigEs8311() {
  if (!status_.es8311.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigEs8311 failed\n");
    return false;
  }

  cpp_bus_driver::Es8311::PowerStatus ps = {
      .contorl =
          {
              .analog_circuits = true,                // 开启模拟电路
              .analog_bias_circuits = true,           // 开启模拟偏置电路
              .analog_adc_bias_circuits = true,       // 开启模拟ADC偏置电路
              .analog_adc_reference_circuits = true,  // 开启模拟ADC参考电路
              .analog_dac_reference_circuit = true,   // 开启模拟DAC参考电路
              .internal_reference_circuits = false,   // 关闭内部参考电路
          },
      .vmid = cpp_bus_driver::Es8311::Vmid::kStartUpVmidNormalSpeedCharge,
  };
  bool result = true;
  result &= chip_.es8311->SetPowerStatus(ps);
  result &= chip_.es8311->SetPgaPower(true);
  result &= chip_.es8311->SetAdcPower(true);
  result &= chip_.es8311->SetDacPower(true);
  result &= chip_.es8311->SetOutputToHpDrive(true);
  result &= chip_.es8311->SetAdcOffsetFreeze(
      cpp_bus_driver::Es8311::AdcOffsetFreeze::kDynamicHpf);
  result &= chip_.es8311->SetAdcHpfStage2Coeff(10);
  result &= chip_.es8311->SetDacEqualizer(false);
  result &= chip_.es8311->SetMic(cpp_bus_driver::Es8311::MicType::kAnalogMic,
      cpp_bus_driver::Es8311::MicInput::kMic1p1n);
  result &= chip_.es8311->SetAdcAutoVolumeControl(false);
  result &=
      chip_.es8311->SetAdcGain(cpp_bus_driver::Es8311::AdcGain::kGain18db);
  result &= chip_.es8311->SetAdcPgaGain(
      cpp_bus_driver::Es8311::AdcPgaGain::kGain30db);
  result &= chip_.es8311->SetAdcVolume(191);
  result &= chip_.es8311->SetDacVolume(191);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigEs8311 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitL76k() {
  if (!chip_.l76k->Init()) {
    if (!bus_.l76k_uart_bus->SetBaudRate(115200)) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "SetBaudRate failed\n");
      return false;
    }
    if (!chip_.l76k->Init()) {
      status_.l76k.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitL76k failed\n");
      return false;
    } else {
      bool result = true;
      result &=
          chip_.l76k->SetBaudRate(cpp_bus_driver::L76k::BaudRate::kBr115200Bps);
      result &= chip_.l76k->SetUpdateFrequency(
          cpp_bus_driver::L76k::UpdateFreq::kFreq5Hz);
      result &= chip_.l76k->ClearRxBufferData();

      status_.l76k.init_flag = result;
      if (result) {
        LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
      } else {
        LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitL76k failed\n");
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

    status_.l76k.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitL76k success\n");
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitL76k failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitIcm20948() {
  if (!chip_.icm20948->init()) {
    if (!chip_.icm20948->initMagnetometer()) {
      status_.icm20948.init_flag = false;
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitIcm20948 failed\n");
      return false;
    } else {
      chip_.icm20948->setAccRange(ICM20948_ACC_RANGE_2G);
      chip_.icm20948->setAccDLPF(ICM20948_DLPF_6);
      chip_.icm20948->setMagOpMode(AK09916_CONT_MODE_20HZ);

      status_.icm20948.init_flag = true;
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitIcm20948 success\n");
      return true;
    }
  } else {
    chip_.icm20948->setAccRange(ICM20948_ACC_RANGE_2G);
    chip_.icm20948->setAccDLPF(ICM20948_DLPF_6);
    chip_.icm20948->setMagOpMode(AK09916_CONT_MODE_20HZ);

    status_.icm20948.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitIcm20948 success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitSx1262() {
  if (!chip_.sx1262->Init(10000000)) {
    status_.sx1262.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitSx1262 failed\n");
    return false;
  } else {
    status_.sx1262.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitSx1262 success\n");
    return true;
  }
}

#if defined CONFIG_BOARD_TYPE_T_DISPLAY_P4_KEYBOARD
bool TDisplayP4Driver::InitXl9555() {
  if (!chip_.xl9555->Init()) {
    status_.xl9555.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitXl9555 failed\n");
    return false;
  } else {
    status_.xl9555.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitXl9555 success\n");
    return true;
  }
}

bool TDisplayP4Driver::ConfigXl9555() {
  if (!status_.xl9555.init_flag) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9555 failed\n");
    return false;
  }

  bool result = true;
  result &= chip_.xl9555->SetGpioMode(
      XL9555_LED_1, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      XL9555_LED_2, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      XL9555_LED_3, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      XL9555_TCA8418_RST, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      XL9555_T_MIXRF_EN, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      XL9555_T_MIXRF_CC1101_RF_SWITCH_0, cpp_bus_driver::Xl95x5::Mode::kOutput);
  result &= chip_.xl9555->SetGpioMode(
      XL9555_T_MIXRF_CC1101_RF_SWITCH_1, cpp_bus_driver::Xl95x5::Mode::kOutput);

  result &= chip_.xl9555->GpioWrite(
      XL9555_LED_1, cpp_bus_driver::Xl95x5::Value::kHigh);  // 关闭led
  result &= chip_.xl9555->GpioWrite(
      XL9555_LED_2, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9555->GpioWrite(
      XL9555_LED_3, cpp_bus_driver::Xl95x5::Value::kHigh);
  result &= chip_.xl9555->GpioWrite(
      XL9555_T_MIXRF_EN, cpp_bus_driver::Xl95x5::Value::kHigh);

  result &= SetCc1101RfSwitch(Cc1101RfSwitch::k868_915Mhz);

  result &= chip_.xl9555->GpioWrite(
      XL9555_TCA8418_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(10);
  result &= chip_.xl9555->GpioWrite(
      XL9555_TCA8418_RST, cpp_bus_driver::Xl95x5::Value::kLow);
  tool_->DelayMs(10);
  result &= chip_.xl9555->GpioWrite(
      XL9555_TCA8418_RST, cpp_bus_driver::Xl95x5::Value::kHigh);
  tool_->DelayMs(10);

  if (!result) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "ConfigXl9555 failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitTca8418() {
  if (!chip_.tca8418->Init()) {
    status_.tca8418.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitTca8418 failed\n");
    return false;
  } else {
    bool result = true;
    result &= chip_.tca8418->SetKeypadScanWindow(
        0, 0, TCA8418_KEYPAD_SCAN_WIDTH, TCA8418_KEYPAD_SCAN_HEIGHT);
    result &= chip_.tca8418->SetIrqGpioMode(
        cpp_bus_driver::Tca8418::IrqMask::kKeyEvents);
    result &= chip_.tca8418->ClearIrqFlag(
        cpp_bus_driver::Tca8418::IrqFlag::kKeyEvents);

    status_.tca8418.init_flag = result;
    if (result) {
      LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitTca8418 success\n");
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__, "InitTca8418 failed\n");
    }
    return result;
  }
}

bool TDisplayP4Driver::InitTca8418Backlight() {
  if (!chip_.tca8418_backlight->Init(ledc_timer_t::LEDC_TIMER_1,
          ledc_channel_t::LEDC_CHANNEL_1, 1000000, 0,
          ledc_mode_t::LEDC_LOW_SPEED_MODE,
          ledc_timer_bit_t ::LEDC_TIMER_5_BIT)) {
    status_.tca8418_backlight.init_flag = false;
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "InitTca8418Backlight failed\n");
    return false;
  } else {
    status_.tca8418_backlight.init_flag = true;
    LogMessage(
        LogLevel::kInfo, __FILE__, __LINE__, "InitTca8418Backlight success\n");
    return true;
  }
}

bool TDisplayP4Driver::InitCc1101() {
  int16_t ret = chip_.cc1101->begin();
  if (ret != RADIOLIB_ERR_NONE) {
    status_.cc1101.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "InitCc1101 failed (error code: %d)\n", ret);
    return false;
  } else {
    status_.cc1101.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitCc1101 success\n");
    return true;
  }
}

bool TDisplayP4Driver::SetCc1101RfSwitch(Cc1101RfSwitch rf_switch) {
  if (!status_.xl9555.init_flag) {
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "SetCc1101RfSwitch failed\n");
    return false;
  }

  bool result = true;
  switch (rf_switch) {
    case Cc1101RfSwitch::k315Mhz:
      result &= chip_.xl9555->GpioWrite(XL9555_T_MIXRF_CC1101_RF_SWITCH_0,
          cpp_bus_driver::Xl95x5::Value::kLow);
      result &= chip_.xl9555->GpioWrite(XL9555_T_MIXRF_CC1101_RF_SWITCH_1,
          cpp_bus_driver::Xl95x5::Value::kHigh);
      break;
    case Cc1101RfSwitch::k434Mhz:
      result &= chip_.xl9555->GpioWrite(XL9555_T_MIXRF_CC1101_RF_SWITCH_0,
          cpp_bus_driver::Xl95x5::Value::kHigh);
      result &= chip_.xl9555->GpioWrite(XL9555_T_MIXRF_CC1101_RF_SWITCH_1,
          cpp_bus_driver::Xl95x5::Value::kHigh);
      break;
    case Cc1101RfSwitch::k868_915Mhz:
      result &= chip_.xl9555->GpioWrite(XL9555_T_MIXRF_CC1101_RF_SWITCH_0,
          cpp_bus_driver::Xl95x5::Value::kHigh);
      result &= chip_.xl9555->GpioWrite(XL9555_T_MIXRF_CC1101_RF_SWITCH_1,
          cpp_bus_driver::Xl95x5::Value::kLow);
      break;

    default:
      result = false;
      break;
  }

  if (!result) {
    LogMessage(
        LogLevel::kChip, __FILE__, __LINE__, "SetCc1101RfSwitch failed\n");
  }
  return result;
}

bool TDisplayP4Driver::InitNrf24l01() {
  int16_t ret = chip_.nrf24l01->begin();
  if (ret != RADIOLIB_ERR_NONE) {
    status_.nrf24l01.init_flag = false;
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "InitNrf24l01 failed (error code: %d)\n", ret);
    return false;
  } else {
    status_.nrf24l01.init_flag = true;
    LogMessage(LogLevel::kInfo, __FILE__, __LINE__, "InitNrf24l01 success\n");
    return true;
  }
}
#endif

bool TDisplayP4Driver::InitSpiffs(
    const char* base_path, esp_vfs_spiffs_conf_t& spiffs_conf) {
  esp_vfs_spiffs_conf_t conf = {
      .base_path = base_path,
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false,
  };

  esp_err_t result = esp_vfs_spiffs_register(&conf);
  if (result != ESP_OK) {
    if (result == ESP_FAIL) {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "Failed to mount or format filesystem (error code: %#X)\n", result);
    } else if (result == ESP_ERR_NOT_FOUND) {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "Failed to find spiffs partition (error code: %#X)\n", result);
    } else {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "Failed to initialize spiffs (error code: %#X)\n", result);
    }
    return false;
  }

  size_t total = 0, used = 0;
  result = esp_spiffs_info(conf.partition_label, &total, &used);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "Failed to get spiffs partition information (error code: %#X). "
        "formatting...\n",
        result);
    esp_spiffs_format(conf.partition_label);
    return false;
  }

  LogMessage(LogLevel::kInfo, __FILE__, __LINE__,
      "Partition size: total: %d bytes, used: %d bytes\n", total, used);

  if (used > total) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "Number of used bytes cannot be larger than total performing "
        "esp_spiffs_check\n");
    result = esp_spiffs_check(conf.partition_label);
    if (result != ESP_OK) {
      LogMessage(LogLevel::kChip, __FILE__, __LINE__,
          "esp_spiffs_check failed (error code: %#X)\n", result);
      return false;
    } else {
      LogMessage(
          LogLevel::kChip, __FILE__, __LINE__, "esp_spiffs_check success\n");
    }
  }

  spiffs_conf = conf;
  return true;
}

bool TDisplayP4Driver::InitSdmmc(const char* base_path, int max_freq_khz) {
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = false,
      .use_one_fat = false,
  };

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.slot = SDMMC_HOST_SLOT_0;
  host.max_freq_khz = max_freq_khz;

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = 4;
  slot_config.clk = static_cast<gpio_num_t>(SD_SDIO_CLK);
  slot_config.cmd = static_cast<gpio_num_t>(SD_SDIO_CMD);
  slot_config.d0 = static_cast<gpio_num_t>(SD_SDIO_D0);
  slot_config.d1 = static_cast<gpio_num_t>(SD_SDIO_D1);
  slot_config.d2 = static_cast<gpio_num_t>(SD_SDIO_D2);
  slot_config.d3 = static_cast<gpio_num_t>(SD_SDIO_D3);
  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

  sdmmc_card_t* card = nullptr;
  esp_err_t result = esp_vfs_fat_sdmmc_mount(
      base_path, &host, &slot_config, &mount_config, &card);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_vfs_fat_sdmmc_mount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    return false;
  }

  sdmmc_card_print_info(stdout, card);

  status_.sd_card.init_flag = true;
  return true;
}

bool TDisplayP4Driver::InitSdspi(
    const char* base_path, spi_host_device_t host_id, int max_freq_khz) {
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = false,
      .use_one_fat = false,
  };

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = host_id;
  host.max_freq_khz = max_freq_khz;

  spi_bus_config_t bus_config = {
      .mosi_io_num = SD_MOSI,
      .miso_io_num = SD_MISO,
      .sclk_io_num = SD_SCLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .data4_io_num = -1,
      .data5_io_num = -1,
      .data6_io_num = -1,
      .data7_io_num = -1,
      .data_io_default_level = 0,
      .max_transfer_sz = 0,
      .flags = SPICOMMON_BUSFLAG_MASTER,
      .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
      .intr_flags = 0,
  };

  esp_err_t result =
      spi_bus_initialize(host_id, &bus_config, SDSPI_DEFAULT_DMA);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "spi_bus_initialize failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    return false;
  }

  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.host_id = host_id;
  slot_config.gpio_cs = static_cast<gpio_num_t>(SD_CS);

  sdmmc_card_t* card = nullptr;
  result = esp_vfs_fat_sdspi_mount(
      base_path, &host, &slot_config, &mount_config, &card);
  if (result != ESP_OK) {
    LogMessage(LogLevel::kChip, __FILE__, __LINE__,
        "esp_vfs_fat_sdspi_mount failed (error code: %#X)\n", result);
    status_.sd_card.init_flag = false;
    return false;
  }

  sdmmc_card_print_info(stdout, card);
  status_.sd_card.init_flag = true;
  return true;
}

}  // namespace lilygo_device_driver
