values = {1: 2, 2: 3}

def get_value(id):
    print "FSX Reading %d -> %f" %(id, values[id])
    return values[id]

def set_value(id, val):
    print "FSX Setting %d -> %f" %(id, val)
    values[id] = val
