#-----------------------------------------------------------------------------
# Run a unit test
#
# Runs the unit test exe and updates a <test>_passed file on success.
#-----------------------------------------------------------------------------
import SCons.Builder
import os
import subprocess
import DeviceTools.telnetrpc

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):

	def builder_unit_test(target, source, env):
		#Emulation unit test
		if env['type'] == 'emulation':
			#Run through each test given
			for t, s in zip(target, source):
				app = str(s.abspath)
				failed_cases = subprocess.call( [ app ], shell=False)
				if failed_cases == 0:
					results = open(str(t), 'w')
					results.write("PASSED\n")
					results.close()
				else:
					results = open(str(t), 'w')
					results.write("FAILED " + str(failed_cases) + " cases\n")
					results.close()
		
		#Embedded unit test
		elif env['type'] == 'embedded':
			#Connect to device and mount nfsroot's LF
			mount_cmd = "mount -o nolock `get-ip host`:" + env['vars']['rootfs'] + " /LF"
			try:
				rpc = DeviceTools.telnetrpc.TelnetRPC("192.168.0.111", "root")
			except Exception:
				for t in target:
					results_log = open(str(t), 'w')
					results_log.write("BLOCKED\n\nFailed to connect to device")
					results_log.close()
					
				raise
			
			rpc.command(mount_cmd, verbose=True)
			
			#Run tests
			#TODO: This way of calculating paths is not very dynamic and elegant
			for t, s in zip(target, source):
				device_test_path = "/LF" + str(s.abspath) [ len( env['vars']['rootfs'] ) : ]
				print("Running: " + device_test_path)
				
				result = rpc.command(device_test_path)
				failed_cases = rpc.command("echo $?")
				
				print("Test Complete")
				
				#Log results
				try:
					results_log = open(str(t), 'w')
				except Exception:
					print("Failed to open results log")
					raise
				
				#Check fail/pass
				if failed_cases[0] == "0":
					results_log.write("PASSED\n\n")
				else:
					results_log.write("FAILED " + failed_cases[0] + " cases\n\n")
				results_log.write("Detailed Log:\n")
				for line in result:
					results_log.write(line + "\n")
				results_log.close()
			
			rpc.command("umount -lf /LF")
			rpc.close()

	bld = SCons.Builder.Builder(action = builder_unit_test)
	env['BUILDERS']['RunTest'] = bld


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
	return 1
