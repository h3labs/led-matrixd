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
beacon = LedMatrixD::Beacon.new iniConfig
LedMatrixD::Native.init
#d1 = LedMatrixD::DisplayI::RandomSpriteDisplay.new iniConfig
#d2 = LedMatrixD::DisplayI::SpinLogoDisplay.new iniConfig
#d3 = LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'title'
#d4 = LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'tagline'
#d5 = LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'twitter'
#d6 = LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'url'
beacon.run
d7 = LedMatrixD::DisplayI::StatusDisplay.new iniConfig, beacon

def shut_down(d)
	d.clean
end

Signal.trap("INT") { 
p "cleaning up before shutdown"
  #shut_down d3
  #shut_down d4
  #shut_down d5
  #shut_down d6
  shut_down d7
beacon.stop
LedMatrixD::Native.clear
  exit
}
 
# Trap `Kill `
Signal.trap("TERM") {
p "cleaning up before shutdown" #shut_down d3
  #shut_down d4
  #shut_down d5
  #shut_down d6
  shut_down d7
beacon.stop
LedMatrixD::Native.clear
  exit
}

while true
	#d3.show
	#d4.show
	#d5.show
	#d6.show
	d7.show
end
sleep(5)
