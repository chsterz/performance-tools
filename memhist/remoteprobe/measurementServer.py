#!/usr/bin/env python2

import sys
import struct
import subprocess
import socket 

class SkipThis(Exception):
	pass

def pack(lat, result):
	return struct.pack(dformat,lat,result)

def measure(lat):
	cmd = 'sudo timeout -s2 0.01s perf stat -a -x, -e cpu/event=0xCD,umask=0x01,ldlat='+str(lat)+'/pp'
	proc = subprocess.Popen(cmd.split(), stderr=subprocess.PIPE)
	s = proc.stderr.read()
	try: 
		return int(s.partition(',')[0])
	except:
		raise SkipThis()

dformat = "<ii"

def serve(PORT=9999):
	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		s.bind(('', PORT))
		s.listen(1)
		while True:
			conn = None
			try:
				conn, addr = s.accept()
				print "Connection from", addr

				lat = 0
				while True:
					try:
						res_lower = measure(lat)
						res_upper = measure(lat + 10)
						diff = res_lower - res_upper
						print lat, ": ", diff
						conn.sendall(pack(lat, diff))
					except SkipThis:
						pass
					lat += 10
					lat %= 500
			except KeyboardInterrupt:
				break
			except Exception, e:
				print e
			finally:
				if conn:
					conn.close()
	finally:
		s.close()

if __name__ == "__main__":
	serve(9999 if len(sys.argv) < 2 else int(sys.argv[1]))
