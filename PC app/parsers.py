def table_to_string(table):
    s = ""
    for i in table:
        s += chr(i)
    return s

def string_to_table(string):
    t = []
    for i in string:
        t.append(ord(i))
    return t


def parse_int32(payload):
    import struct
    st = table_to_string(payload)
    x = struct.unpack("i", st)
    return x[0]


def parse_bool(payload):
    import struct
    st = table_to_string(payload)
    x = struct.unpack("?", st)
    return x[0]


def parse_float(payload):
    import struct
    st = table_to_string(payload)
    x = struct.unpack("f", st)
    return x[0]


def encode_float(value):
    import struct
    x = struct.pack("f", value)
    return string_to_table(x)


def parse_null(payload):
    return payload
