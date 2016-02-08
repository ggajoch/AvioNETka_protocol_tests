from DataStructs import *

class PHYInterface:
    def passDown(self, data: PHYDataStruct) -> None:
        pass

class NETInterface:
    def passUp(self, data: PHYDataStruct) -> None:
        pass

    def passDown(self, data: NETDataStruct) -> None:
        pass

class FSXInterface:
    def passUp(self, data: NETDataStruct) -> None:
        pass
