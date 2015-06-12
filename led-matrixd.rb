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

iniConfig = IniFile.load('matrix.ini')
#beacon = LedMatrixD::Beacon.new iniConfig
LedMatrixD::Native.init
d1 = LedMatrixD::DisplayI::RandomSpriteDisplay.new iniConfig
while true
	d1.show
end
sleep(5)
#beacon.run
#beacon.join
