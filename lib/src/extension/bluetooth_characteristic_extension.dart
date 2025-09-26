import 'package:flutter_blue_plus_platform_interface/flutter_blue_plus_platform_interface.dart';
import 'package:flutter_blue_plus_windows/flutter_blue_plus_windows.dart';

extension BluetoothCharacteristicExtension on BluetoothCharacteristic {
  BmBluetoothCharacteristic toProto(int instanceId) {
    return BmBluetoothCharacteristic(
      instanceId: instanceId,
      remoteId: DeviceIdentifier(remoteId.str),
      serviceUuid: serviceUuid,
      characteristicUuid: characteristicUuid,
      descriptors: [for (final d in descriptors) d.toProto(instanceId)],
      properties: properties.toProto(),
      primaryServiceUuid: null, // TODO:  API changes
    );
  }
}
