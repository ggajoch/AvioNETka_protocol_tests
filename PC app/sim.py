class SimVar:
    def __init__(self, id_):
        self.subscriptions = []
        self.id = id_
        self.val = None

    def subscribe(self, handler):
        self.subscriptions.append(handler)

    def set(self, value: float):
        print("[SIM] Setting %d to" % self.id, value)
        if self.val != value:
            self.val = value
            for i in self.subscriptions:
                i(self.val)

SimVariables = {}


def set_subscription(handler, fsx_id: int):
    if fsx_id not in SimVariables:
        SimVariables[fsx_id] = SimVar(fsx_id)

    SimVariables[fsx_id].subscribe(handler)


def set(id, val):
    SimVariables[id].set(val)


def get(id):
    return SimVariables[id].value
