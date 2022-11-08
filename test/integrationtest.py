import json
import subprocess
import time
import sys
import os
from testutils import run_efu, run_data_generator, get_metrics

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
from utils.efushell.EFUMetrics import Metrics


def compare_pair(x, y, operator):
    if operator == "==":
        return x == y
    if operator == ">=":
        return x >= y
    raise Exception(f"invalid operator {operator}")


def check_stats(test):
    print("Getting EFU Stats")
    metrics = get_metrics()

    for stats_test in test['StatsTestList']:
        actual_stat = metrics.return_metric(f"efu.{test['Module']}.0.{stats_test[0]}")
        operator = stats_test[1]
        expected_stat = stats_test[2]
        if actual_stat == -1:
        	# metrics.return_metric returns -1 if named stat doesn't exist
            raise Exception(
                f"EFU stat check failed, did not find stat named {stats_test[0]}"
            )
        if not compare_pair(actual_stat, expected_stat, operator):
            raise Exception(
                f"EFU stat check failed, {actual_stat} {operator} {expected_stat}"
            )
        else:
            print(
                f"Stat check passed for {stats_test[0]} {stats_test[1]} {stats_test[2]}"
            )


def create_kafka_topic(topic_name):
    subprocess.Popen(
        f"/ess/ecdc/kafka/kafka_2.13-2.8.0/bin/kafka-topics.sh --bootstrap-server localhost:9092 --delete --topic {topic_name}",
        shell=True,
    ).wait()
    subprocess.Popen(
        f"/ess/ecdc/kafka/kafka_2.13-2.8.0/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic {topic_name}",
        shell=True,
    ).wait()


def check_kafka(test):
    kafka_process = subprocess.Popen(
        f"/ess/ecdc/kafka/kafka_2.13-2.8.0/bin/kafka-verifiable-consumer.sh --bootstrap-server localhost:9092 --topic {test['KafkaTopic']} --group-id testconsumer1",
        shell=True,
        stdout=subprocess.PIPE,
    )
    # waiting 5 seconds for kafka consumer to consume all messages
    time.sleep(60)
    kafka_process.kill()
    out, err = kafka_process.communicate()
    results_dict = [
        json.loads(str(x)) for x in out.decode("utf-8").split("\n") if str(x) != ""
    ]
    messages = 0
    for result in results_dict:
        if result['name'] == 'records_consumed':
            messages += result['count']
    if messages == test['ExpectedKafkaMessages']:
        print(
            f"Successfully received {test['ExpectedKafkaMessages']} messages in Kafka topic {test['KafkaTopic']}"
        )
    else:
        raise Exception(
            f"Did not successfully receive {test['ExpectedKafkaMessages']} messages in Kafka topic {test['KafkaTopic']}, received {messages}"
        )


def run_tests():
    efu = "./event-formation-unit"

    with open('./test/integrationtest.json') as f:
        data = json.load(f)

    for test in data['Tests']:
        create_kafka_topic(test['KafkaTopic'])
        efu_process = run_efu(efu, test['Module'], test['Config'])
        try:
            run_data_generator(efu, test['Generator'], test['Packets'], test['Throttle'])
            time.sleep(1)
            run_data_generator(efu, test['Generator'], test['Packets'], test['Throttle'])
            check_stats(test)
            check_kafka(test)
            efu_process.kill()
        except:
            efu_process.kill()
            raise


if __name__ == "__main__":
    run_tests()
