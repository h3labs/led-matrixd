#!/usr/bin/env ruby
# script to control led-matrixd demon

require 'daemons'
options = {
	:backtrace => true,
	:log_dir => '/var/log/',
	:log_output => true
}
Daemons.run('/var/led-matrixd/led-matrixd.rb', options)
