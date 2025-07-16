import 'package:flutter/services.dart';

/// An [IsolateActivator] encapuslates information to make
/// the library's initialization also available for non-root
/// isolates. This Object has to be created in the root-isolate
/// and then passed onto the background isolate manually.
class IsolateActivator {
  /// The underlying native-executable file as bytes.
  final ByteData bytes;

  /// The Root Isolate Token to enable specialized function
  /// calls from the background isolate.
  final RootIsolateToken rit;

  IsolateActivator(this.bytes, this.rit);
}
