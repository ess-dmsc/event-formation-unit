#!/usr/local/bin/python3.9

from PyQt5.QtWidgets import *
from PyQt5 import QtCore
from os.path import expanduser
import sys, os, re, subprocess, signal, configparser
import argparse


# Reads configuration from file or use reasonable default values
class Configuration:
    def __init__(self, profile):
        self.profile = profile
        self.options = {}
        self.config = configparser.RawConfigParser()
        self.config.read(os.path.join(expanduser("~"), ".efucfg"))
        self._load("qtefu_latest", "efudir")
        self._load("qtefu_latest", "datadir")
        self._load("servers", "grafana_"+profile)
        self._load("servers", "kafka_"+profile)
        self._load("qtefu_latest", "grafana")
        self._load("qtefu_latest", "kafka")
        self._load("qtefu_latest", "hwcheck")
        self._load("qtefu_latest", "region")

    # load option or use defult value
    def _load(self, group, opt):
        if self.config.has_option(group, opt):
            self.options[opt] = self.config.get(group, opt)
        else:
            print("No option {}, using blank".format(opt))
            self.options[opt] = ""

    #  updates the [qtefu_latest] section of the ~/.efucfg file
    def update_latest(self, selection):
        for name, value in selection.items():
            self.config.set("qtefu_latest", name, value)
        self.write_config()

    def get_latest(self, name):
        if self.config.has_option("qtefu_latest", name):
            return self.config.get("qtefu_latest", name)
        else:
            return ""

    # writes current config settings to ~/.efucfg file
    def write_config(self):
        with open(os.path.join(expanduser("~"), ".efucfg"), "w") as configfile:
            self.config.write(configfile)


# search for relevant files specified by regexp
class Searcher:
    # omit files matching 'exclude', then add files matching 'match'
    # since our filenaming is somewhat inconsistent there might be
    # false positives.
    def find_files(self, dir, match, exclude):
        results = []
        print("Searching directory: " + dir)
        for r, s, f in os.walk(dir):
            if re.search(exclude, r):
                continue
            for file in f:
                if re.search(match, file):
                    filepath = os.path.join(r, file)
                    results += [os.path.relpath(filepath, dir)]                
        return results

    # Here we search for efu binary, module plugins (.so), config and calib files (.json)
    def get_values(self, cfg):
        return [
            self.find_files(os.path.join(cfg.options["efudir"], "bin"), "", "-X--xXX"),
            [""] + self.find_files(cfg.options["datadir"], "\.json", "build"),
            [""] + self.find_files(cfg.options["datadir"], ".*calib.*\.json", "build"),
        ]


#
# #
#
class Dialog(QDialog):  # WMainWindow
    def __init__(self, configuration):
        super(QDialog, self).__init__()
        self.cfg = configuration
        self.create_layout()

        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.on_accepted)
        buttonBox.rejected.connect(self.reject)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.efu_group_box)
        mainLayout.addWidget(self.config_group_box)
        mainLayout.addWidget(self.options_box)
        mainLayout.addWidget(buttonBox)
        self.setLayout(mainLayout)

        self.setWindowTitle("Event Formation Unit (EFU) Launcher")

    # Add a 'row' consisting of a label and a text box widget
    def add_row(self, layout, label, type):
        lbl = QLabel(label)
        type.setMinimumWidth(600)
        layout.addRow(lbl, type)

    def create_layout(self):
        self.efu_group_box = QGroupBox("Select EFU")
        toplayout = QFormLayout()
        self.efudirle = QLineEdit()
        self.efudirle.textChanged.connect(self.update)
        self.add_row(toplayout, "efu dir:", self.efudirle)
        self.detcb = QComboBox()
        self.add_row(toplayout, "Detector:", self.detcb)
        self.efu_group_box.setLayout(toplayout)

        self.config_group_box = QGroupBox("Select configuration")
        fileslayout = QFormLayout()
        self.datadirle = QLineEdit()
        self.datadirle.textChanged.connect(self.update)
        self.add_row(fileslayout, "data dir:", self.datadirle)
        self.cfgcb = QComboBox()
        self.add_row(fileslayout, "Config:", self.cfgcb)
        self.calcb = QComboBox()
        self.add_row(fileslayout, "Calib:", self.calcb)
        self.config_group_box.setLayout(fileslayout)

        self.options_box = QGroupBox("Options")
        optslayout = QFormLayout()
        self.grafanale = QLineEdit(cfg.options["grafana_" + self.cfg.profile])
        self.add_row(optslayout, "Grafana IP:", self.grafanale)
        self.kafkale = QLineEdit(cfg.options["kafka_" + self.cfg.profile])
        self.add_row(optslayout, "Kafka IP:", self.kafkale)
        self.hwcheckle = QLineEdit(cfg.options["hwcheck"])
        self.add_row(optslayout, "HW check:", self.hwcheckle)
        self.regionle = QLineEdit(cfg.options["region"])
        self.add_row(optslayout, "Region:", self.regionle)
        self.options_box.setLayout(optslayout)

    def _populate_field(self, field, list):
        for name in list:
            field.addItem(name)

    def populate(self, detector, config, calib):
        self._populate_field(self.detcb, detector)
        self._populate_field(self.cfgcb, config)
        self._populate_field(self.calcb, calib)

    # returns currently selected options as a dictionary
    def get_selection(self):
        return {
            "efudir": self._efudir,
            "datadir": self._datadir,
            "det": self._det,
            "config": self._cfg,
            "calib": self._cal,
            "grafana": self._grafana,
            "kafka": self._kafka,
            "hwcheck": self._hwcheck,
            "region": self._region,
        }

    # Override builtin on_accepted method for pressing OK button
    def on_accepted(self):
        self._efudir = self.efudirle.text()
        self._datadir = self.datadirle.text()
        self._det = self.detcb.itemText(self.detcb.currentIndex())
        self._cfg = self.cfgcb.itemText(self.cfgcb.currentIndex())
        self._cal = self.calcb.itemText(self.calcb.currentIndex())
        self._grafana = self.grafanale.text()
        self._kafka = self.kafkale.text()
        self._hwcheck = self.hwcheckle.text()
        self._region = self.regionle.text()
        self.accept()

    def update(self):
        self.detcb.clear()
        self.cfgcb.clear()
        self.calcb.clear()
        search = Searcher()
        self.cfg.options["efudir"] = self.efudirle.text()
        self.cfg.options["datadir"] = self.datadirle.text()
        detector, config, calib = search.get_values(self.cfg)
        self.populate(detector, config, calib)

    def set_defaults(self):
        self.efudirle.setText(self.cfg.get_latest("efudir"))
        self.datadirle.setText(self.cfg.get_latest("datadir"))
        det_index = self.detcb.findText(self.cfg.get_latest("det"))
        self.detcb.setCurrentIndex(det_index)
        cfg_index = self.cfgcb.findText(
           self.cfg.get_latest("config")
        )
        self.cfgcb.setCurrentIndex(cfg_index)
        cal_index = self.calcb.findText(
            self.cfg.get_latest("calib")
        )
        self.calcb.setCurrentIndex(cal_index)


# runs efu command with given directories and configuration selection
def run_cmdlopts(selection):
    cmdlopts = [
        os.path.join(selection["efudir"], "bin", selection["det"]),
    ]
    if selection["hwcheck"] == "False":
        cmdlopts += ["--nohwcheck"]
    if selection["grafana"] != "":
        cmdlopts += ["--graphite", selection["grafana"]]
    if selection["kafka"] != "":
        cmdlopts += ["--broker_addr", selection["kafka"]]
    if selection["config"] != "":
        cmdlopts += ["--file", os.path.join(selection["datadir"], selection["config"])]
    if selection["calib"] != "":
        cmdlopts += [
            "--calibration",
            os.path.join(selection["datadir"], selection["calib"]),
        ]
    if selection["region"] != "":
        cmdlopts += ["--region", selection["region"]]
    print(" ".join(cmdlopts))
    subprocess.call(cmdlopts)


#
#
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-p",
        metavar="profile",
        help="profiles: grafana_profile",
        type=str,
        default="none",
    )
    parser.add_argument(
        "-r",
        "--resume",
        help="resume efu with previously used parameters",
        action="store_true",
    )
    args = parser.parse_args()
    cfg = Configuration(args.p)

    # if resume argument used, uses saved parameters in ~/.efucfg
    if args.resume:
        selection = dict(cfg.config.items("qtefu_latest"))
        run_cmdlopts(selection)
    # else loads GUI for parameter selection
    else:
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        app = QApplication(sys.argv)

        searcher = Searcher()
        dialog = Dialog(cfg)

        detector, config, calib = searcher.get_values(cfg)
        dialog.populate(detector, config, calib)

        dialog.set_defaults()

        retval = dialog.exec_()
        if retval != 0:
            selection = dialog.get_selection()
            cfg.update_latest(selection)
            run_cmdlopts(selection)
    sys.exit()
