#-----------------------------------------------------------------------------
# Run a unit test
#
# Runs the unit test exe and updates a <test>_passed file on success.
#-----------------------------------------------------------------------------
import SCons.Builder
import os
import subprocess
import signal
import DeviceTools.telnetrpc

class TestTimeoutError(Exception):
	pass

def TestTimeoutHandler(signum, frame):
	raise TestTimeoutError

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):

	def builder_unit_test(target, source, env):
		#Emulation unit test
		if env['vars']['is_emulation']:
			#Run through each test given
			for t, s in zip(target, source):
				app = str(s.abspath)
				
				#Startup test and create a timer
				result = []
				test_process = subprocess.Popen( [ app ], shell = True, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
				signal.signal(signal.SIGALRM, TestTimeoutHandler)
				signal.alarm(300)
				
				#Read lines from the pipe until the test finishes or times out
				try:
					while (test_process.returncode == None):
						result.append( test_process.stdout.readline() )
						if result[-1] == "":
							break
						print(result[-1])
					signal.alarm(0)
				except TestTimeoutError:
					appname = os.path.basename(app)
					os.system( "killall " + str(appname) )
					result.append("Test Terminated Due To Timeout")
					test_process.wait()
					
				#Collect result data
				failed_cases = test_process.returncode
				results_log = open(str(t), 'w')
				if failed_cases == 0:
					results_log.write("PASSED\n\n")
				else:
					results_log.write("FAILED " + str(failed_cases) + " cases\n\n")
				results_log.write("Detailed Log:\n")
				for line in result:
					results_log.write( line )
				results_log.close()
		
		#Embedded unit test
		else:
			#Connect to device and mount nfsroot's LF
			mount_cmd = "mount -o nolock `get-ip host`:" + env['vars']['rootfs'] + " /LF"
			try:
				rpc = DeviceTools.telnetrpc.TelnetRPC("192.168.0.111", "root")
			except Exception:
				for t in target:
					results_log = open(str(t), 'w')
					results_log.write("BLOCKED\n\nFailed to connect to device")
					results_log.close()
				return
			
			try:
				rpc.command(mount_cmd, verbose=True)
			except Exception:
				for t in target:
					results_log = open(str(t), 'w')
					results_log.write("BLOCKED\n\nFailed to mount nfsroot")
					results_log.close()
				return
			
			#Run tests
			#TODO: This way of calculating paths is not very dynamic and elegant
			for t, s in zip(target, source):
				#Log results
				try:
					results_log = open(str(t), 'w')
				except Exception:
					print("Failed to open results log")
					continue
				
				device_test_path = "/LF" + str(s.abspath) [ len( env['vars']['rootfs'] ) : ]
				
				try:
					result = rpc.command(device_test_path)
					failed_cases = rpc.command("echo $?")
				except Exception:
					results_log.write("BLOCKED\nFailed to connect to device\n\n")
					results_log.close()
					continue
					
				#Check fail/pass
				return_code = int(failed_cases[0])
				if return_code == 0:
					results_log.write("PASSED\n\n")
				elif return_code > 128:
					results_log.write("BROKEN\nTest caught signal "+str(return_code - 128) + "\n\n")
				else:
					results_log.write("FAILED " + return_code + " cases\n\n")
				results_log.write("Detailed Log:\n")
				for line in result:
					results_log.write(line + "\n")
				results_log.close()
			
			try:
				rpc.command("umount -lf /LF")
				rpc.close()
			except Exception:
				return

	bld = SCons.Builder.Builder(action = builder_unit_test)
	env['BUILDERS']['RunTest'] = bld


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
	return 1
