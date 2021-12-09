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
        self._load("directories", "basedir")
        self._load("directories", "efudir")
        self._load("directories", "datadir")
        self._load("servers", "grafana_" + profile)
        self._load("servers", "kafka_" + profile)
        self._load("efuopts", "hwcheck")
        self._load("efuopts", "region")

    # load option or use defult value
    def _load(self, group, opt):
        if self.config.has_option(group, opt):
            self.options[opt] = self.config.get(group, opt)
        else:
            print("No option {}, exiting ...".format(opt))
            sys.exit(0)

    #  updates the [qtefu_latest] section of the ~/.efucfg file
    def update_latest(self, selection):
        for name, value in selection.items():
            self.config.set("qtefu_latest", name, value)
        self.write_config()

    #  updates the [directories] section of the ~/.efucfg file
    def update_dirs(self, dirs):
        self.config.set("directories", "basedir", dirs.basedir)
        self.config.set("directories", "datadir", dirs.datadir)
        self.config.set("directories", "efudir", dirs.efudir)
        self.write_config()

    # writes current config settings to ~/.efucfg file
    def write_config(self):
        with open(os.path.join(expanduser("~"), ".efucfg"), "w") as configfile:
            self.config.write(configfile)


# Maintain a set of directories for searching
class Directories:
    def __init__(self, base, efu, data):
        self.set_dirs(base, efu, data)

    def set_dirs(self, base, efu, data):
        self.basedir = base
        self.efudir = efu
        self.datadir = data
        self.searchdirs = [self.basedir + self.datadir, self.basedir + self.efudir]


# search for relevant files specified by regexp
class Searcher:
    def __init__(self, dirs):
        self.dirs = dirs

    # omit files matching 'exclude', then add files matching 'match'
    # since our filenaming is somewhat inconsistent there might be
    # false positives.
    def find_files(self, dirs, match, exclude):
        results = []
        for dir in dirs:
            for r, s, f in os.walk(dir):
                if re.search(exclude, r):
                    continue
                for file in f:
                    if re.search(match, file):
                        filepath = os.path.join(r, file)
                        results += [os.path.relpath(filepath, self.dirs.basedir)]
        return results

    # Here we search for efu binary, module plugins (.so), config and calib files (.json)
    def get_values(self):
        return [
            self.find_files(self.dirs.searchdirs, "efu$", "-X--xXX"),
            self.find_files(self.dirs.searchdirs, "\.so", "-X--xXX"),
            [""] + self.find_files(self.dirs.searchdirs, "\.json", "build"),
            [""] + self.find_files(self.dirs.searchdirs, ".*calib.*\.json", "build"),
        ]


#
# #
#
class Dialog(QDialog):  # WMainWindow
    def __init__(self, configuration, directories):
        super(QDialog, self).__init__()
        self.dirs = directories
        self.cfg = configuration
        self.create_layout()

        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttonBox.accepted.connect(self.on_accepted)
        buttonBox.rejected.connect(self.reject)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.config_group_box)
        mainLayout.addWidget(self.files_group_box)
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
        self.config_group_box = QGroupBox("Directories")
        toplayout = QFormLayout()

        self.basedirle = QLineEdit(self.dirs.basedir)
        self.add_row(toplayout, "basedir:", self.basedirle)
        self.efudirle = QLineEdit(self.dirs.efudir)
        self.add_row(toplayout, "efu dir:", self.efudirle)
        self.datadirle = QLineEdit(self.dirs.datadir)
        self.add_row(toplayout, "data dir:", self.datadirle)
        updateb = QPushButton("Reload")
        updateb.setMaximumWidth(100)
        updateb.clicked.connect(self.update)
        toplayout.addRow(updateb)
        self.config_group_box.setLayout(toplayout)

        self.files_group_box = QGroupBox("Select configuration")
        fileslayout = QFormLayout()
        self.efucb = QComboBox()
        self.add_row(fileslayout, "EFU:", self.efucb)
        self.detcb = QComboBox()
        self.add_row(fileslayout, "Detector:", self.detcb)
        self.cfgcb = QComboBox()
        self.add_row(fileslayout, "Config:", self.cfgcb)
        self.calcb = QComboBox()
        self.add_row(fileslayout, "Calib:", self.calcb)
        self.files_group_box.setLayout(fileslayout)
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

    def populate(self, efu, detector, config, calib):
        self._populate_field(self.efucb, efu)
        self._populate_field(self.detcb, detector)
        self._populate_field(self.cfgcb, config)
        self._populate_field(self.calcb, calib)

    # returns currently selected options as a dictionary
    def get_selection(self):
        return {
            "efu": self._efu,
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
        self._efu = self.efucb.itemText(self.efucb.currentIndex())
        self._det = self.detcb.itemText(self.detcb.currentIndex())
        self._cfg = self.cfgcb.itemText(self.cfgcb.currentIndex())
        self._cal = self.calcb.itemText(self.calcb.currentIndex())
        self._grafana = self.grafanale.text()
        self._kafka = self.kafkale.text()
        self._hwcheck = self.hwcheckle.text()
        self._region = self.regionle.text()
        self.accept()

    def update(self):
        self.efucb.clear()
        self.detcb.clear()
        self.cfgcb.clear()
        self.calcb.clear()
        self.dirs.set_dirs(
            self.basedirle.text(), self.efudirle.text(), self.datadirle.text()
        )
        search = Searcher(self.dirs)
        efu, detector, config, calib = search.get_values()
        self.populate(efu, detector, config, calib)

    def set_defaults(self, cfg):
        efu_index = self.efucb.findText(
            dict(self.cfg.config.items("qtefu_latest"))["efu"]
        )
        self.efucb.setCurrentIndex(efu_index)
        det_index = self.detcb.findText(
            dict(self.cfg.config.items("qtefu_latest"))["det"]
        )
        self.detcb.setCurrentIndex(det_index)
        cfg_index = self.cfgcb.findText(
            dict(self.cfg.config.items("qtefu_latest"))["config"]
        )
        self.cfgcb.setCurrentIndex(cfg_index)
        cal_index = self.calcb.findText(
            dict(self.cfg.config.items("qtefu_latest"))["calib"]
        )
        self.calcb.setCurrentIndex(cal_index)


# runs efu command with given directories and configuration selection
def run_cmdlopts(dirs, selection):
    cmdlopts = [
        os.path.join(dirs.basedir, selection["efu"]),
        "--det",
        os.path.join(dirs.basedir, selection["det"]),
    ]
    if selection["hwcheck"] == "False":
        cmdlopts += ["--nohwcheck"]
    if selection["grafana"] != "":
        cmdlopts += ["--graphite", selection["grafana"]]
    if selection["kafka"] != "":
        cmdlopts += ["--broker_addr", selection["kafka"]]
    if selection["config"] != "":
        cmdlopts += ["--file", os.path.join(dirs.basedir, selection["config"])]
    if selection["calib"] != "":
        cmdlopts += [
            "--calibration",
            os.path.join(dirs.basedir, selection["calib"]),
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
        default="office",
    )
    parser.add_argument(
        "-r",
        "--resume",
        help="resume efu with previously used parameters",
        action="store_true",
    )
    args = parser.parse_args()
    cfg = Configuration(args.p)
    dirs = Directories(
        cfg.options["basedir"], cfg.options["efudir"], cfg.options["datadir"]
    )

    # if resume argument used, uses saved parameters in ~/.efucfg
    if args.resume:
        selection = dict(cfg.config.items("qtefu_latest"))
        run_cmdlopts(dirs, selection)
    # else loads GUI for parameter selection
    else:
        signal.signal(signal.SIGINT, signal.SIG_DFL)
        app = QApplication(sys.argv)

        searcher = Searcher(dirs)
        dialog = Dialog(cfg, dirs)

        efu, detector, config, calib = searcher.get_values()
        dialog.populate(efu, detector, config, calib)

        dialog.set_defaults(cfg)

        retval = dialog.exec_()
        if retval != 0:
            selection = dialog.get_selection()
            cfg.update_latest(selection)
            cfg.update_dirs(dirs)
            run_cmdlopts(dirs, selection)
    sys.exit()
