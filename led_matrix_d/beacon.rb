
module LedMatrixD
	class Beacon
		def initialize(iniConfig)
			@filename = iniConfig["FILE SYSTEM"]["shop_status_flag"]
			@updateScript = iniConfig["FILE SYSTEM"]["shop_status_update_script"]
			@restoreScript = iniConfig["FILE SYSTEM"]["shop_status_restore_script"]
			@beaconInfo = ThreadSafe::Hash.new
			@thread = nil
			readBeaconFile
		end
		def run
			#watch the filename	
			@thread = Thread.new do	
				notifier = INotify::Notifier.new
				basename = File.basename(@filename)
				dirWatcher = notifier.watch(File.dirname(@filename), :create, :attrib, :modify) do |event|
					if basename == event.name
						$logger.info "got event flags " + event.flags.to_s
						$logger.info "for file " + event.name
						readBeaconFile
					end
				end
				while true
					if IO.select([notifier.to_io], nil, nil, nil)
						notifier.process				
					end
				end
			end
		end
		def readBeaconFile
			beaconFile = File.open(@filename, "r")
			beaconContent = beaconFile.read
			beaconFile.close
			$logger.info beaconContent
			unless beaconContent.nil?
				if beaconContent[-1,1] == "\n"
					beaconContent = beaconContent[0, beaconContent.size - 1]
				end
				CGI::parse(beaconContent).each do |key, val|
					@beaconInfo[key] = val[0]
				end
			end
			$logger.info @beaconInfo.to_s
		end
		def stop
			$logger.info "stopping beacon thread"
			$logger.info @beaconInfo.to_s
			@thread.kill
		end
		def getInfo key = nil
			return @beaconInfo[key]
		end
	end
end
