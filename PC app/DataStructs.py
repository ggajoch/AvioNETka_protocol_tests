class PHYDataStruct:
    def __init__(self):
        self.address = 0
        self.data = []

    def __str__(self):
        return "(%d; ", str(self.data), ")"


class NETDataStruct:
    def __init__(self):
        self.address = 0
        self.command = 0
        self.data = []

