template <typename T>
ArduinoCloudProperty<T>::ArduinoCloudProperty(T& _property, T const _shadow_property, String const & _name, T const _minDelta, permissionType const _permission, long const _updatePolicy, void(*fn)(void)) :
  ArduinoCloudPropertyGeneric(fn),
  property       (_property       ),
  shadow_property(_shadow_property),
  name           (_name           ),
  minDelta       (_minDelta       ),
  permission     (_permission     ),
  updatePolicy   (_updatePolicy   )
{
}

template <typename T>
bool ArduinoCloudProperty<T>::write(T value) {
  /* permissions are intended as seen from cloud */
  if (canWrite()) {
    property = value;
    return true;
  }
  return false;
}

template <typename T>
T ArduinoCloudProperty<T>::read() {
  /* permissions are intended as seen from cloud */
  if (canRead()) {
    return property;
  }
  /* FIXME: What happens if we can not read? Compiler should complain there
   * because there is no return value in case of the canRead() evaluating
   * to false
   */
}

template <typename T>
void ArduinoCloudProperty<T>::printinfo(Stream& stream) {
  stream.println("name: " + name + " value: " + String(property) + " shadow: " + String(shadow_property) + " permission: " + String(permission));
}

template <typename T>
bool ArduinoCloudProperty<T>::shouldBeUpdated() const {
    if (updatePolicy == ON_CHANGE) {
        return newData();
    }
    return ((millis() - lastUpdated) > (updatePolicy * 1000)) ;
}

template <>
inline bool ArduinoCloudProperty<int>::newData() const {
    return (property != shadow_property && abs(property - shadow_property) >= minDelta );
}

template <>
inline bool ArduinoCloudProperty<float>::newData() const {
    return (property != shadow_property && abs(property - shadow_property) >= minDelta );
}

template <typename T>
void ArduinoCloudProperty<T>::append(CborEncoder* encoder) {
  if (!canRead()) {
    return;
  }
  CborEncoder mapEncoder;
  cbor_encoder_create_map(encoder, &mapEncoder, CborIndefiniteLength);
  if (tag != -1) {
    cbor_encode_text_stringz(&mapEncoder, "t");
    cbor_encode_int(&mapEncoder, tag);
  }
  else {
    cbor_encode_text_stringz(&mapEncoder, "n");
    cbor_encode_text_stringz(&mapEncoder, name.c_str());
  }
  appendValue(&mapEncoder);
  cbor_encoder_close_container(encoder, &mapEncoder);
  lastUpdated = millis();
}

// Different appendValue function for different property typer, because the CBOR encoder and message format
template <>
inline void ArduinoCloudProperty<int>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_int(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "vb");
    cbor_encode_boolean(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "v");
    cbor_encode_float(mapEncoder, property);
};

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborEncoder* mapEncoder) {
    cbor_encode_text_stringz(mapEncoder, "vs");
    cbor_encode_text_stringz(mapEncoder, property.c_str());
};

// Return property type
template <>
inline propertyType ArduinoCloudProperty<int>::getType() const {
    return INT;
}

template <>
inline propertyType ArduinoCloudProperty<float>::getType() const {
    return FLOAT;
}

template <>
inline propertyType ArduinoCloudProperty<bool>::getType() const {
    return BOOL;
}

template <>
inline propertyType ArduinoCloudProperty<String>::getType() const {
    return STRING;
}
