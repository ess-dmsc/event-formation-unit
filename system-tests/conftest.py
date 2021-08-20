import os.path
import pytest
from compose.cli.main import TopLevelCommand, project_from_options
from confluent_kafka import Producer
import docker


def pytest_addoption(parser):

    def _is_valid_file(arg, filename):
        if not os.path.isfile(os.path.join(arg, filename)):
            raise Exception("The file {} does not exist at the given path {}".format(filename, arg))
        else:
            return arg

    parser.addoption("--pcap-file-path", type=lambda x: _is_valid_file(x, "ess2_ess_mask.pcap"),
                     help="Path of directory containing pcap file (ess2_ess_mask.pcap)", required=True)
    parser.addoption("--json-file-path", type=lambda x: _is_valid_file(x, "MB18Freia.json"),
                     help="Path of directory containing JSON file of detector details (MB18Freia.json)", required=True)


def _wait_until_kafka_ready(docker_cmd, docker_options):
    print('Waiting for Kafka broker to be ready for system tests...')
    conf = {'bootstrap.servers': 'localhost:9094',
            'api.version.request': True}
    producer = Producer(**conf)
    kafka_ready = False

    def delivery_callback(err, msg):
        nonlocal n_polls
        nonlocal kafka_ready
        if not err:
            print('Kafka is ready!')
            kafka_ready = True

    n_polls = 0
    while n_polls < 10 and not kafka_ready:
        producer.produce('waitUntilUp', value='Test message', on_delivery=delivery_callback)
        producer.poll(10)
        n_polls += 1

    if not kafka_ready:
        docker_cmd.down(docker_options)  # Bring down containers cleanly
        raise Exception('Kafka broker was not ready after 100 seconds, aborting tests.')

log_options = {   "--no-color": False,
                  "--follow": False,
                  "--timestamps": False,
                  "--tail": "all"
                  }

common_options = {"--no-deps": False,
                  "--always-recreate-deps": False,
                  "--scale": "",
                  "--abort-on-container-exit": False,
                  "SERVICE": "",
                  "--remove-orphans": False,
                  "--no-recreate": True,
                  "--force-recreate": False,
                  '--no-build': False,
                  '--no-color': False,
                  "--rmi": "none",
                  "--volumes": True,  # Remove volumes when docker-compose down (don't persist kafka and zk data)
                  "--follow": False,
                  "--timestamps": False,
                  "--tail": "all",
                  "--detach": True,
                  "--build": False
                  }


def _build_efu_image():
    client = docker.from_env()
    print("Building EFU image", flush=True)
    build_args = {}
    if "http_proxy" in os.environ:
        build_args["http_proxy"] = os.environ["http_proxy"]
    if "https_proxy" in os.environ:
        build_args["https_proxy"] = os.environ["https_proxy"]
    image, logs = client.images.build(path="../", tag="event-formation-unit:latest", rm=False, buildargs=build_args)
    for item in logs:
        print(item, flush=True)


def _run_containers(cmd, options):
    print("Running docker-compose up", flush=True)
    cmd.up(options)
    print("\nFinished docker-compose up\n", flush=True)
    _wait_until_kafka_ready(cmd, options)


def _build_and_run(options, request):
    _build_efu_image()
    project = project_from_options(os.path.dirname(__file__), options)
    cmd = TopLevelCommand(project)
    _run_containers(cmd, options)

    def fin():
        # Stop the containers then remove them and their volumes (--volumes option)
        print("containers stopping", flush=True)
        log_ops = dict(log_options)
        cmd.logs(log_opts)
        options["--timeout"] = 30
        cmd.down(options)
        print("containers stopped", flush=True)

    # Using a finalizer rather than yield in the fixture means
    # that the containers will be brought down even if tests fail
    request.addfinalizer(fin)


def _remove_trailing_path_delimiter(path):
    if path[-1] in ['/', '\\']:
        path = path[:-1]
    return path


def _create_docker_compose_file(request):
    """
    Populate data directory fields in the docker-compose-template file and output
    a usable docker-compose script
    """
    pcap_file = _remove_trailing_path_delimiter(request.config.getoption("--pcap-file-path"))
    json_file = _remove_trailing_path_delimiter(request.config.getoption("--json-file-path"))

    with open('docker-compose-template.yml', 'r+') as dc_template:
        dc_content = dc_template.read()
        dc_content = dc_content.replace("PCAP_DATA_PATH", pcap_file)
        dc_content = dc_content.replace("JSON_DATA_PATH", json_file)
        with open('docker-compose.yml', 'w+') as dc_new:
            dc_new.write(dc_content)


@pytest.fixture(scope="module")
def docker_compose(request):
    """
    :type request: _pytest.python.FixtureRequest
    """
    print("Started preparing test environment...", flush=True)

    _create_docker_compose_file(request)

    # Options must be given as long form
    options = common_options
    options["--file"] = ["docker-compose.yml"]
    return _build_and_run(options, request)
