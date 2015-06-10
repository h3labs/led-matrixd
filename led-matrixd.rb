require 'thread'
require 'thread_safe'
require 'rb-inotify'
require 'inifile'
require 'cgi'


Dir[File.join(File.dirname(__FILE__), 'led_matrix_d','**' ,'*.rb')].each do |file|
	p file.to_s
	require file
end

iniConfig = IniFile.load('matrix.ini')
beacon = LedMatrixD::Beacon.new iniConfig
beacon.run
beacon.join
