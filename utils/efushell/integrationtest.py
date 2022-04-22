import json
import subprocess
import time
from EFUMetrics import Metrics


def compare_pair(x, y, operator):
    if operator == "==":
        return x == y
    if operator == ">=":
        return x >= y
    print("invalid operator")
    return False


def run_efu(test, efu):
	print("Running EFU")
	efu_command = f"{efu}/bin/efu --det {efu}/modules/{test['Module']}.so --nohwcheck --file {test['Config']} --region 0 --graphite 127.0.0.1"
	print(efu_command)
	efu_process = subprocess.Popen(f"exec {efu_command}", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	time.sleep(5)
	poll = efu_process.poll()
	if poll is not None:
		out, errs = efu_process.communicate()
		print(out)
		print(errs)
		raise Exception(f"Running efu with command {efu_command} failed")
	return efu_process


def run_data_generator(test, efu):
	print("Running Data Generator")
	generator_process = subprocess.Popen(f"{efu}/generators/{test['Generator']} -a {test['Packets']} -o 50 -t {test['Throttle']}", shell=True)
	exit_code = generator_process.wait()
	if exit_code != 0:
		raise Exception(f"Artififial generator {test['Generator']} failed")
	time.sleep(5)
	return generator_process


def check_stats(test, stats_test_list):
	print("Getting EFU Stats")
	
	for i in range(10):
		try:
			metrics = Metrics('127.0.0.1', 8888)
			metrics.get_all_metrics(metrics.get_number_of_stats())
			break
		except:
			print("failed to get metrics")

	for stats_test in stats_test_list:
		actual_stat = metrics.return_metric(f"efu.{test['Module']}.0.{stats_test[0]}")
		operator = stats_test[1]
		expected_stat = stats_test[2]
		if actual_stat == -1:
			efu_process.kill()
			raise Exception(f"EFU stat check failed, did not find stat named {stats_test[0]}")
		if not compare_pair(actual_stat, expected_stat, operator):
			efu_process.kill()
			raise Exception(f"EFU stat check failed, {actual_stat} {operator} {expected_stat}")
		else:
			print(f"Stat check passed for {stats_test[0]} {stats_test[1]} {stats_test[2]}")



def create_kafka_topic(topic_name):
        subprocess.Popen(f"/ess/ecdc/kafka/kafka_2.13-2.8.0/bin/kafka-topics.sh --bootstrap-server localhost:9092 --create --topic {topic_name}", shell=True).wait()

def check_kafka(test):
        kafka_process = subprocess.Popen(f"/ess/ecdc/kafka/kafka_2.13-2.8.0/bin/kafka-verifiable-consumer.sh --bootstrap-server localhost:9092 --topic {test["KafkaTopic"]} --group-id testconsumer1", shell=True, stdout=subprocess.PIPE)
        time.sleep(5)
        kafka_process.kill()
        out, err = kafka_process.communicate()
        results_dict = [json.loads(str(x)) for x in out.decode("utf-8").split("\n") if str(x) != '']
        messages = 0
        for result in results_dict:
                if result['name'] == "records_consumed":
                        messages += result['count']
        if messages == test["ExpectedKafkaMessages"]:
                print(f"Successfully received {test["ExpectedKafkaMessages"]} messages in Kafka topic {test["KafkaTopic"]}")
        else:
                raise Exception(f"Did not successfully receive {test["ExpectedKafkaMessages"]} messages in Kafka topic {test["KafkaTopic"]}, received {messages}")

def run_tests():
        efu = "./event-formation-unit"
        stats_test_list = [
                                                ['kafka.ev_errors', '==', 0],
                                                ['transmit.bytes', '>=', 500000]
        ]

        file = open('./utils/efushell/integrationtest.json')
        data = json.load(file)
        file.close()


        for test in data['Tests']:
                create_kafka_topic(test["KafkaTopic"])
                efu_process = run_efu(test, efu)
                try:
                        run_data_generator(test, efu)
                        time.sleep(1)
                        run_data_generator(test, efu)
                        check_stats(test, stats_test_list)
                        check_kafka(test)
                        efu_process.kill()
                except:
                        efu_process.kill()
                        raise Exception()



if __name__ == "__main__":
	run_tests()