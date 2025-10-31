# Changelog

**Generated on:** 2025-09-30 12:54:20
**Commit range:** `1.3.4` to `2.0.0-alpha.2`
**Total commits:** 30

## 🎫 JIRA Issues

- [ECDC-4296](https://jira.ess.eu/browse/ECDC-4296)
- [ECDC-4892](https://jira.ess.eu/browse/ECDC-4892)
- [ECDC-4893](https://jira.ess.eu/browse/ECDC-4893)
- [ECDC-4915](https://jira.ess.eu/browse/ECDC-4915)
- [ECDC-4964](https://jira.ess.eu/browse/ECDC-4964)
- [ECDC-4972](https://jira.ess.eu/browse/ECDC-4972)
- [ECDC-4990](https://jira.ess.eu/browse/ECDC-4990)
- [ECDC-5045](https://jira.ess.eu/browse/ECDC-5045)
- [ECDC-5080](https://jira.ess.eu/browse/ECDC-5080)

## ✨ Features

- **ECDC-5080 Implement changelog generator script and update documentation** ([ECDC-5080](https://jira.ess.eu/browse/ECDC-5080)) - `2aef48a5` by Tibor Bukovics
- **ECDC-4972 Implement thread safety for Statistics, prevent parser to value copy mainstats** ([ECDC-4972](https://jira.ess.eu/browse/ECDC-4972)) - `a21ba45e` by Tibor Bukovics

## 🐛 Bug Fixes

- **ECDC-4915 Correct OQ stats name** ([ECDC-4915](https://jira.ess.eu/browse/ECDC-4915)) - `35a56e79` by Tibor Bukovics
- **ECDC-4892: Fix readout time for MultiBladeGenerator** ([ECDC-4892](https://jira.ess.eu/browse/ECDC-4892)) - `95b5a2ca` by Mads Ipsen

## ♻️ Refactoring

- **Refactor grafana to root and update compose files. Add new blue efu dashboard** - `42e4da2c` by Tibor Bukovics
- **ECDC-4990 Introduce StatCounterBase and refactor core parts and common metrics** ([ECDC-4990](https://jira.ess.eu/browse/ECDC-4990)) - `4b24757a` by Tibor Bukovics
- **ECDC-4915 improve efu Statistics prefix handling with store prefix in stat tuple. Refactor stats in detector, correct missing stats. Align tests with changes** ([ECDC-4915](https://jira.ess.eu/browse/ECDC-4915)) - `bde08101` by Tibor Bukovics
- **ECDC-4893 refactor caen efu config** ([ECDC-4893](https://jira.ess.eu/browse/ECDC-4893)) - `12de0581` by Tibor Bukovics
- **ECDC-4296 1st step refactor ESSParser and RefTime statistics counter** ([ECDC-4296](https://jira.ess.eu/browse/ECDC-4296)) - `56f9bbd6` by Tibor Bukovics

## 🧪 Tests

- **Improve test stability with reorder calls in writePacketToRxFIFO** - `ef403b4c` by Tibor Bukovics
- **Add Dream unit configs from essconfig for local test and debug** - `2f42023e` by Tibor Bukovics

## 🔧 Maintenance

- **Update the dashboard with old nmx metrics** - `ae0dd706` by Tibor Bukovics
- **update ci to release docker image from tags** - `c9a1a5e6` by Tibor Bukovics

## 📝 General Changes

- **Remove unused Grafana provisioning files and Jenkins scripts** - `07c4cf15` by Tibor Bukovics
- **ECDC-5045 efu banjo compose utgaard** ([ECDC-5045](https://jira.ess.eu/browse/ECDC-5045)) - `46472b36` by Tibor Bukovics
- **Added functionality for sum up pulses in histogram** - `377a1a14` by Michael Christiansen
- **Updated how serialized buffer is used for histogram and how it is returned...** - `f3bd3b6c` by Michael Christiansen
- **data generator for DREAM STF data** - `d1c971c9` by Morten Jagd Christensen
- **Mch/ecdc 4913 pixel error bugfix** - `90cae657` by Michael Christiansen
- **Added beam mask functionality to CBM 2D using 4 space invader figures and on...** - `db407778` by Michael Christiansen
- **ECDC-4964 overflow handling for dream improvements on dream generator for SUMO6** ([ECDC-4964](https://jira.ess.eu/browse/ECDC-4964)) - `19495d44` by Tibor Bukovics
- **Mchr/ecdc 4680 develop 2 d cbm efu** - `0f139018` by Michael Christiansen
- **Initial version of a 2D CBM using uniform distribution of (x, y) dimension,...** - `3a10003d` by Michael Christiansen
- **add -z option to efustats.py** - `ffffafc6` by Morten Jagd Christensen
- **std::tie didn't work for all compilers** - `0c64dbd6` by Morten Jagd Christensen
- **add generateReadoutTimeEveryN() function** - `d63a8c83` by Morten Jagd Christensen
- **ignore devcontainer and not the gitmodules** - `a8b9fad0` by Tibor Bukovics
- **Moved socket mock to common testutil, Enabled AmplitudeMask on Caen generator and add tests** - `1980fa6a` by Michael Christiansen
- **- Renaming variable** - `588f58d1` by Mads Ipsen
- **Change previous pulse time to be calculated as Current pulse time subtracted...** - `5d028379` by Michael Christiansen
