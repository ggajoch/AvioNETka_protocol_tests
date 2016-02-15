import struct


def encode(encoding: int, data):
    if encoding == 0:
        return encode_bool(int(data))
    elif encoding == 1:
        return encode_float(float(data))
    elif encoding == 2:
        return encode_uint8(int(data))
    elif encoding == 3:
        return encode_uint16(int(data))
    elif encoding == 4:
        return encode_uint32(int(data))
    elif encoding == 5:
        return encode_int8(int(data))
    elif encoding == 6:
        return encode_int16(int(data))
    elif encoding == 7:
        return encode_int32(int(data))


def parse(encoding: int, data):
    if encoding == 0:
        return parse_bool(data)
    elif encoding == 1:
        return parse_float(data)
    elif encoding == 2:
        return parse_uint8(data)
    elif encoding == 3:
        return parse_uint16(data)
    elif encoding == 4:
        return parse_uint32(data)
    elif encoding == 5:
        return parse_int8(data)
    elif encoding == 6:
        return parse_int16(data)
    elif encoding == 7:
        return parse_int32(data)


def encode_var_name(name, value):
    x = struct.pack(name, value)
    return x


def parse_var_name(name, payload):
    x = struct.unpack(name, payload)
    return x[0]


def parse_uint8(payload):
    return parse_var_name("B", payload)


def encode_uint8(value):
    return encode_var_name("B", value)


def parse_uint16(payload):
    return parse_var_name("H", payload)


def encode_uint16(value):
    return encode_var_name("H", value)


def parse_uint32(payload):
    return parse_var_name("I", payload)


def encode_uint32(value):
    return encode_var_name("I", value)


def parse_int8(payload):
    return parse_var_name("b", payload)


def encode_int8(value):
    return encode_var_name("b", value)


def parse_int16(payload):
    return parse_var_name("h", payload)


def encode_int16(value):
    return encode_var_name("h", value)


def parse_int32(payload):
    return parse_var_name("i", payload)


def encode_int32(value):
    return encode_var_name("i", value)


def parse_bool(payload):
    return parse_var_name("?", payload)


def encode_bool(value):
    return encode_var_name("?", value)


def parse_float(payload):
    return parse_var_name("f", payload)


def encode_float(value):
    return encode_var_name("f", value)


def parse_null(payload):
    return payload
