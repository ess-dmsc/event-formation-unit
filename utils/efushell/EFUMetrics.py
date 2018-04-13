from SocketDriver import SimpleSocket

class Metrics:
    def __init__(self, ip, port):
        self.metrics = {}
        self.ip = ip
        self.port = port
        self.driver = SimpleSocket(self.ip, self.port)

    def getEFUCommand(self, cmd):
        res = self.driver.Ask(cmd)

        if res.find("Error") != -1:
            print("Error getting EFU command")
            sys.exit(1)
        return res

    def getNumberOfStats(self):
        return int(self.getEFUCommand('STAT_GET_COUNT').split()[1])

    def getAllMetrics(self, num_metrics):
        for i in range(num_metrics):
            res =  self.getEFUCommand('STAT_GET ' + str(i + 1)).split()
            name = res[1]
            value = int(res[2])
            self.metrics[name] = value

    def compareMetric(self, name, value):
        try:
            res = (self.metrics[name] == value)
        except:
            return False

        return res
