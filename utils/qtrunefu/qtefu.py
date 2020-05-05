#!/usr/local/bin/python3.7

from PyQt5.QtWidgets import *
from PyQt5 import QtCore
from os.path import expanduser
import sys, os, re, subprocess, signal, configparser


# Reads configuration from file or use reasonable default values
class Configuration:
    def __init__(self):
        self.options = {}
        self.config = configparser.RawConfigParser()
        self.config.read(os.path.join(expanduser("~"), ".efucfg"))
        self._load('directories', 'basedir', expanduser("~"))
        self._load('directories', 'efudir', '/essproj/event-formation-unit')
        self._load('directories', 'datadir', '/ownCloud/DM/data/EFU_reference')
        self._load('servers', 'grafana', '172.17.12.31')
        self._load('servers', 'kafka', '172.17.5.38:9092')
        self._load('efuopts', 'hwcheck', False)

    # load option or use defult value
    def _load(self, group, opt, default):
        if self.config.has_option(group, opt):
            self.options[opt] = self.config.get(group, opt)
        else:
            self.options[opt] = default


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
        return [ self.find_files(self.dirs.searchdirs, 'efu$', '-X--xXX'),
                 self.find_files(self.dirs.searchdirs, '\.so', '-X--xXX'),
                 [''] + self.find_files(self.dirs.searchdirs, '\.json', 'build'),
                 [''] + self.find_files(self.dirs.searchdirs, '.*calib.*\.json', 'build') ]
#
# #
#
class Dialog(QDialog): #WMainWindow
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
        self.grafanale = QLineEdit(cfg.options['grafana'])
        self.add_row(optslayout, "Grafana IP:", self.grafanale)
        self.kafkale = QLineEdit(cfg.options['kafka'])
        self.add_row(optslayout, "Kafka IP:", self.kafkale)
        self.hwcheckle = QLineEdit(cfg.options['hwcheck'])
        self.add_row(optslayout, "HW check:", self.hwcheckle)
        self.options_box.setLayout(optslayout)

    def _populate_field(self, field, list):
        for name in list:
            field.addItem(name)

    def populate(self, efu, detector, config, calib):
        self._populate_field(self.efucb, efu)
        self._populate_field(self.detcb, detector)
        self._populate_field(self.cfgcb, config)
        self._populate_field(self.calcb, calib)

    def get_selection(self):
        return self._efu, self._det, self._cfg, self._cal, self._grafana, self._kafka, self._hwcheck

    # Override builtin on_accepted method for pressing OK button
    def on_accepted(self):
        self._efu = self.efucb.itemText(self.efucb.currentIndex())
        self._det = self.detcb.itemText(self.detcb.currentIndex())
        self._cfg = self.cfgcb.itemText(self.cfgcb.currentIndex())
        self._cal = self.calcb.itemText(self.calcb.currentIndex())
        self._grafana = self.grafanale.text()
        self._kafka = self.kafkale.text()
        self._hwcheck = self.hwcheckle.text()
        self.accept()

    def update(self):
        self.efucb.clear()
        self.detcb.clear()
        self.cfgcb.clear()
        self.calcb.clear()
        self.dirs.set_dirs(self.basedirle.text(), self.efudirle.text(), self.datadirle.text())
        search = Searcher(self.dirs)
        efu, detector, config, calib = search.get_values()
        self.populate(efu, detector, config, calib)

#
#
if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app = QApplication(sys.argv)

    cfg = Configuration()
    dirs = Directories(cfg.options['basedir'], cfg.options['efudir'], cfg.options['datadir'])
    searcher = Searcher(dirs)
    dialog = Dialog(cfg, dirs)

    efu, detector, config, calib = searcher.get_values()
    dialog.populate(efu, detector, config, calib)

    retval = dialog.exec_()
    if retval != 0:
        cmdlopts=[]
        efu, det, config, calib, grafana, kafka, hwcheck = dialog.get_selection()
        if hwcheck == "False":
            cmdlopts += ['--nohwcheck']
        if grafana != "":
            cmdlopts += ['--graphite', grafana]
        if kafka != "":
            cmdlopts += ['--broker_addr', kafka]
        if config != "":
            cmdlopts += ['--file', os.path.join(dirs.basedir, config)]
        if calib != "":
            cmdlopts += ['--calibration', os.path.join(dirs.basedir, calib)]
        subprocess.call([os.path.join(dirs.basedir, efu), '--det', os.path.join(dirs.basedir, det)] + cmdlopts)
    sys.exit()
