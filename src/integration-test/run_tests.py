import json
import subprocess
import time


def get_stat(stats_output, stat_name):
	for stat in stats_output:
		if stat_name in stat:
			return stat.split(' ')[-1]
	raise Exception(f"stat {stat_name} doesn't exist in stats_output")


def run_tests():
	efu = "/Users/jenniferwalker/Documents/essproj/event-formation-unit/build"
	stats_test_list = [['kafka.ev_errors', '0']]

	file = open('tests.json')
	data = json.load(file)
	file.close()


	for test in data['Tests']:
		print("Running EFU")
		efu_command = f"{efu}/bin/efu --det {efu}/modules/{test['Module']}.so --nohwcheck --file {test['Config']} --region 0 --graphite 172.30.242.21"
		print(efu_command)
		efu_process = subprocess.Popen(f"exec {efu_command}", shell=True)
		time.sleep(5)
		poll = efu_process.poll()
		if poll is not None:
			raise Exception(f"Running efu with command {efu_command} failed")

		print("Running Data Generator")
		generator_process = subprocess.Popen(f"{efu}/generators/{test['Generator']} -a {test['Packets']} -o 50 -t {test['Throttle']}", shell=True)
		exit_code = generator_process.wait()
		if exit_code != 0:
			raise Exception(f"Artififial generator {test['Generator']} failed")
		time.sleep(1)

		print("Getting EFU Stats")
		stats_process = subprocess.Popen(f"exec python3 ../../utils/efushell/efustats.py", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		time.sleep(1)
		stats_output, err = stats_process.communicate()
		stats_output = str(stats_output).split('\\n')
		for stats_test in stats_test_list:
			actual_stat = get_stat(stats_output, stats_test[0])
			expected_stat = stats_test[1]
			if actual_stat != expected_stat:
				efu_process.kill()
				raise Exception(f"EFU stat check failed, got {actual_stat} and expected {expected_stat}")
			else:
				print(f"Stat check passed for {stats_test[0]} == {stats_test[1]}")


		efu_process.kill()

if __name__ == "__main__":
	run_tests()