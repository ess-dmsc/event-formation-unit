## event-formation-unit system tests 

### How to run the tests

[optional] Set up a Python virtual environment and activate it (see [here](https://virtualenv.pypa.io/en/stable/))
* Install Docker 

* Install the requirements using pip: `pip install -r system-tests/requirements.txt`

* Download system test data files (`ess2_ess_mask.pcap` and `MB18Freia.json`) from ESS OwnCloud.
You will need to request access to the `data` folder, the files can then be found
at `data/EFU_reference/multiblade/2018_11_22`. 

* Run the following from the `system-tests/` directory:
```
pytest -s --pcap-file-path <PATH_TO_DIR_CONTAINING_PCAP_FILE> --json-file-path <PATH_TO_DIR_CONTAINING_JSON_FILE> .
```
