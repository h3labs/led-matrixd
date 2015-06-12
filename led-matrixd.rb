require 'thread'
require 'thread_safe'
require 'rb-inotify'
require 'inifile'
require 'ffi'
require 'cgi'
require_relative 'led_matrix_d/display.rb'


Dir[File.join(File.dirname(__FILE__), 'led_matrix_d','**' ,'*.rb')].each do |file|
	p file.to_s
	require file
end


iniConfig = IniFile.load('matrix.ini')
#beacon = LedMatrixD::Beacon.new iniConfig
LedMatrixD::Native.init
#d1 = LedMatrixD::DisplayI::RandomSpriteDisplay.new iniConfig
d2 = LedMatrixD::DisplayI::SpinLogoDisplay.new iniConfig
#beacon.run
#beacon.join

def shut_down(d)
	p "cleaning up before shutdown"
	LedMatrixD::Native.clear
	d.clean
end

Signal.trap("INT") { 
  shut_down d2
  exit
}
 
# Trap `Kill `
Signal.trap("TERM") {
  shut_down d2
  exit
}

while true
	d2.show
end
sleep(5)
