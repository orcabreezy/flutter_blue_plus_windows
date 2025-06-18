#include "flutter_blue_plus_windows_plugin.h"
#include "winrt/Windows.Devices.Radios.h"

#include <windows.h>
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include <map>
#include <string>
#include <vector>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Radios;

namespace flutter_blue_plus_windows {
enum class LogLevel {
  LNONE = 0,
  LERROR = 1,
  LWARNING = 2,
  LINFO = 3,
  LDEBUG = 4,
  LVERBOSE = 5,
};

enum class AdapterState {
  Unknown = 0,
  On = 4,
  Off = 6,
};

LogLevel current_log_level = LogLevel::LNONE;

void FlutterBluePlusWindowsPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_blue_plus/methods",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterBluePlusWindowsPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterBluePlusWindowsPlugin::FlutterBluePlusWindowsPlugin() {}

FlutterBluePlusWindowsPlugin::~FlutterBluePlusWindowsPlugin() {}

fire_and_forget GetSystemDevicesAsync(std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    try {
      auto selector = BluetoothDevice::GetDeviceSelector();
      auto additionalProperties = winrt::single_threaded_vector<winrt::hstring>();
      additionalProperties.Append(L"System.Devices.Aep.IsConnected");
      additionalProperties.Append(L"System.Devices.Aep.SignalStrength");
      additionalProperties.Append(L"System.Devices.Aep.IsPaired");
      additionalProperties.Append(L"System.Devices.Aep.DeviceAddress");

      DeviceInformationCollection deviceInfoCollection = co_await DeviceInformation::FindAllAsync(selector, additionalProperties);
      flutter::EncodableMap response = {};
      flutter::EncodableList deviceList;

      for (auto&& deviceInfo : deviceInfoCollection) {
        try {
          auto properties = deviceInfo.Properties();
          flutter::EncodableMap deviceMap = {};
          std::string name = to_string(deviceInfo.Name());
          if (!properties.HasKey(L"System.Devices.Aep.IsConnected") ||
              !unbox_value<bool>(properties.Lookup(L"System.Devices.Aep.IsConnected"))) continue;
          if (name.empty() && properties.HasKey(L"System.Devices.Aep.DeviceAddress")) {
            name = to_string(unbox_value<hstring>(properties.Lookup(L"System.Devices.Aep.DeviceAddress")));
          }
          std::string full_id = to_string(deviceInfo.Id());
          std::string remote_id = full_id.substr(full_id.find_last_of('-') + 1);
          std::transform(remote_id.begin(), remote_id.end(), remote_id.begin(),
            [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
          deviceMap[flutter::EncodableValue("remote_id")] = flutter::EncodableValue(remote_id);
          deviceMap[flutter::EncodableValue("platform_name")] = flutter::EncodableValue(name);
          deviceList.push_back(flutter::EncodableValue(deviceMap));
        } catch (const hresult_error& e) {
          OutputDebugStringW(L"Error processing device: ");
          OutputDebugStringW(e.message().c_str());
          OutputDebugStringW(L"\n");
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
      }

      response[flutter::EncodableValue("devices")] = deviceList;
      result->Success(flutter::EncodableValue(response));
    } catch (const hresult_error& e) {
      result->Error("getSystemDevices", to_string(e.message()));
    } catch (const std::exception& e) {
      result->Error("getSystemDevices", e.what());
    }
    co_return;
}

fire_and_forget GetAdapterStateAsync(std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
     try {
         auto radios = co_await Radio::GetRadiosAsync();
         std::string adapter_name = "";
         int adapter_state = 0;
         for (auto radio : radios) {
             if (radio.Kind() != RadioKind::Bluetooth) continue;
             adapter_name = to_string(radio.Name());
             switch (radio.State()) {
                 case RadioState::On:
                     adapter_state = static_cast<int>(AdapterState::On);
                     break;
                 case RadioState::Off:
                     adapter_state = static_cast<int>(AdapterState::Off);
                     break;
                 default:
                     adapter_state = static_cast<int>(AdapterState::Unknown);
                     break;
             }
             break;
         }
         flutter::EncodableMap response = {};
         response[flutter::EncodableValue("adapter_state")] = flutter::EncodableValue(adapter_state);
         response[flutter::EncodableValue("adapter_name")] = flutter::EncodableValue(adapter_name);
         result->Success(flutter::EncodableValue(response));
     } catch (const hresult_error& e) {
         result->Error("getAdapterState", to_string(e.message()));
     } catch (const std::exception& e) {
         result->Error("getAdapterState", e.what());
     }
    co_return;
}

void FlutterBluePlusWindowsPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const auto& method = method_call.method_name();

  if (method == "flutterRestart") {
    result->Success(0);
    return;
  }

  if (method == "setOptions") {
      result->Success(true);
      return;
  }

  if (method == "connectedCount") {
    result->Success(0);
    return;
  }

  if (method == "setLogLevel") {
    const auto* arguments = std::get_if<int>(method_call.arguments());
    if (arguments == nullptr) {
      result->Error("setLogLevel", "Invalid argument type");
      return;
    }
    int level = *arguments;
    switch (level) {
      case 0:
        current_log_level = LogLevel::LNONE;
        break;
      case 1:
        current_log_level = LogLevel::LERROR;
        break;
      case 2:
        current_log_level = LogLevel::LWARNING;
        break;
      case 3:
        current_log_level = LogLevel::LINFO;
        break;
      case 4:
        current_log_level = LogLevel::LDEBUG;
        break;
      case 5:
        current_log_level = LogLevel::LVERBOSE;
        break;
      default:
        current_log_level = LogLevel::LNONE;
        break;
    }
    result->Success(true);
    return;
  }

  if (method == "isSupported") {
    try {
      auto radios_op = Radio::GetRadiosAsync();
      auto radios = radios_op.get();
      bool supported = false;
      for (auto radio : radios) {
        if (radio.Kind() == RadioKind::Bluetooth) {
          supported = true;
          break;
        }
      }
      result->Success(flutter::EncodableValue(supported));
    } catch (const hresult_error& e) {
      result->Error("isSupported", to_string(e.message()));
    } catch (const std::exception& e) {
      result->Error("isSupported", e.what());
    }
    return;
  }

  if (method == "getAdapterName") {
  }

  if (method == "getAdapterState") {
     try {
       GetAdapterStateAsync(std::move(result));
     } catch (const std::exception& e) {
       result->Error("getAdapterState", e.what());
     }
    return;
  }

  if (method == "turnOn") {

  }

  if (method == "turnOff") {

  }

  if (method == "startScan") {

  }

  if (method == "stopScan") {
  }

  if (method == "getSystemDevices") {
    try {
      GetSystemDevicesAsync(std::move(result));
    } catch (const std::exception& e) {
      result->Error("getSystemDevices", e.what());
    }
    return;
  }

  if (method == "connect") {
  }

  if (method == "disconnect") {
  }

  if (method == "discoverServices") {
  }

  if (method == "readCharacteristic") {
  }

  if (method == "writeCharacteristic") {
  }

  if (method == "readDescriptor") {
  }

  if (method == "writeDescriptor") {
  }

  if (method == "setNotifyValue") {
  }

  if (method == "requestMtu") {
  }

  if (method == "readRssi") {
  }

  if (method == "requestConnectionPriority") {
  }

  if (method == "getPhySupport") {
  }

  if (method == "setPreferredPhy") {
  }

  if (method == "getBondedDevices") {
  }

  if (method == "getBondState") {
  }

  if (method == "createBond") {
  }

  if (method == "removeBond") {
  }

  if (method == "clearGattCache") {
  }

  result->NotImplemented();
}

}