## Fork to add the ability to use bluetooth on windows from a non-root Isolate

This fork adds some logic to be able to use `FlutterBluePlus` from a background isolate. To do this one has to create an activation Object, containing necessary information, to in the **root-isolate**:
```dart
final IsolateActivator activator = await FlutterBluePlusWindows.prepareIsolateActivation();
```
This activation object then has to be send somehow to the background isolate:
```dart
// at creation:
await Isolate.run(() => bluetoothFunction(activator));

// or via a send port
sendPort.send(activator);
```

In the **background isolate** use:
```dart
// e.g. get the activator object via a receive stream:
final IsolateActivator activator = await receivePortStream.first as IsolateActivator;
// activate the library
await FlutterBluerPlusWindows.activateIsolate(activator);
```

---
---

[![pub package](https://img.shields.io/pub/v/flutter_blue_plus_windows.svg)](https://pub.dartlang.org/packages/flutter_blue_plus_windows)

## Flutter Blue Plus Windows

This project is a wrapper library for `Flutter Blue Plus` and `Win_ble`.
It allows `Flutter_blue_plus` to operate on Windows.

With minimal effort, you can use Flutter Blue Plus on Windows.

## Usage
Only you need to do is change the import statement.

```dart
// instead of import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_blue_plus_windows/flutter_blue_plus_windows.dart';

// Alternatively, you can hide FlutterBluePlus when importing the FBP statement
import 'package:flutter_blue_plus/flutter_blue_plus.dart' hide FlutterBluePlus;
import 'package:flutter_blue_plus_windows/flutter_blue_plus_windows.dart';

```

### Scan devices
```dart
final scannedDevices = <ScanResult>{};

const timeout = Duration(seconds: 3);
FlutterBluePlus.startScan(timeout: timeout);

final sub = FlutterBluePlus.scanResults.expand((e)=>e).listen(scannedDevices.add);

await Future.delayed(timeout);
sub.cancel();
scannedDevices.forEach(print);
```

### Connect a device
```dart
final scannedDevice = scannedDevices
    .where((scanResult) => scanResult.device.platformName == DEVICE_NAME)
    .firstOrNull;
final device = scannedDevice?.device;
device?.connect();
```

### Disconnect the device
```dart
device?.disconnect();
```

Check out the usage of Flutter Blue Plus on [Flutter Blue Plus](https://pub.dev/packages/flutter_blue_plus)



