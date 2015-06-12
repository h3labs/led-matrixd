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

#load configuration
iniConfig = IniFile.load('matrix.ini')
if iniConfig.nil? 
	p "could not find 'matrix.ini' configuration file"
end
#initiate beacon file checking thread
beacon = LedMatrixD::Beacon.new iniConfig
beacon.run
#initiate the matrix
LedMatrixD::Native.init
#initiate all displays and store in array
displays = []
displays.push(LedMatrixD::DisplayI::RandomSpriteDisplay.new iniConfig)
displays.push(LedMatrixD::DisplayI::SpinLogoDisplay.new iniConfig)
displays.push(LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'title')
displays.push(LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'tagline')
displays.push(LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'twitter')
displays.push(LedMatrixD::DisplayI::ScrollImageDisplay.new iniConfig, 'url')
displays.push(LedMatrixD::DisplayI::StatusDisplay.new iniConfig, beacon)

def shut_down(disps)
	disps.each do |display|
		p "Cleaning #{display.class.to_s}"
		display.clean
	end
end

Signal.trap("INT") { 
	p "cleaning up before shutdown"
	shut_down(displays)
	beacon.stop
	LedMatrixD::Native.clear
	exit
}
 
# Trap `Kill `
Signal.trap("TERM") {
	p "TERM cleaning up before shutdown" #shut_down d3
	shut_down(displays)
	beacon.stop
	LedMatrixD::Native.clear
	exit
}
#run indefinitly
while true
	displays.each do |display|
		display.show
	end
end
