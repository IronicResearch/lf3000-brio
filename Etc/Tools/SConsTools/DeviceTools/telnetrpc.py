"""
Run remote calls on the device via a telnet connection.

"""

import telnetlib
import re

class TelnetRPC:
	prompt = ''
	session = None
	
	def __init__(self, host="", username="", password=""):
		if host != "":
			self.connect(host, username, password)
	
	def connect(self, host, username="", password=""):
		if self.session != None:
			return
		#Set expected prompt regex
		if username == "root":
			self.prompt = [re.compile("~.*#"), re.compile("/.*#")]
		else:
			self.prompt = [re.compile("~.*\$"), re.compile("/.*\$")]
		
		#connect the session
		try:
			self.session = telnetlib.Telnet(host, timeout = 5)
		except Exception:
			print("Failed to connect to device")
			self.session = None
			raise
		
		#Wait for login prompt (or 5 sec)
		result = self.session.read_until(":", 5)
		
		if result[0] == -1:
			raise IOError("No login found")
			
		#Login
		self.session.write(username+"\n")
		
		if password != "":
			result = self.session.read_until(":", 5)
		
			if result[0] == -1:
				print "TelnetRPC: Didn't ask for password as expected, continuing anyways"
		
		self.session.expect(self.prompt)

	def command(self, cmd, verbose=False):
		"""Run the command on the remote connection.  Return the result as a string."""
		if not self.session:
			print "Not Connected"
			raise IOError("Not connected")
		
		cmd = cmd.strip()
		if verbose:
			print "#", cmd
		
		self.session.write(cmd + "\n")
		try:
			result = self.session.expect(self.prompt, timeout=600)
		except Exception as (err):
			print("Telnet RPC Error: " + str(err))
			raise
		
		feedback = result[2].replace('\r\n', '\n')
		feedback = feedback.split('\n')[1:-1]
		if verbose:
			for line in feedback:
			    print ">>>", line
		return feedback

	def close(self):
		if self.session != None:
			self.session.close()
			self.session = None
