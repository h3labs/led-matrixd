require 'thread'
require 'thread_safe'
require 'rb-inotify'
require 'inifile'
require 'ffi'
require 'cgi'


Dir[File.join(File.dirname(__FILE__), 'led_matrix_d','**' ,'*.rb')].each do |file|
	p file.to_s
	require file
end

#iniConfig = IniFile.load('matrix.ini')
#beacon = LedMatrixD::Beacon.new iniConfig
ptr = FFI::MemoryPointer.new :pointer
LedMatrixD::Native.init
LedMatrixD::Native.load_image "something", ptr
#beacon.run
#beacon.join
