
def getUint16(table):
    return ((table[1] << 8) | table[0]);


def getUint32(table):
    return ((table[3] << 24) | (table[2] << 16) | (table[1] << 8) | table[0]);

def setUint16(val):
    return [val >> 8, val & 0xFF]


def setUint32(table, val):
    return [(val >> 24) & 0xFF, (val >> 16) & 0xFF, (val >> 8) & 0xFF, val & 0xFF]
