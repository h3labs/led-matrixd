
module LedMatrixD
	class Beacon
		def initialize(iniConfig)
			@filename = iniConfig["FILE SYSTEM"]["shop_status_flag"]
			@updateScript = iniConfig["FILE SYSTEM"]["shop_status_update_script"]
			@restoreScript = iniConfig["FILE SYSTEM"]["shop_status_restore_script"]
			@beaconInfo = ThreadSafe::Hash.new
			@thread = nil
		end
		def run
			#watch the filename	
			@thread = Thread.new do	
				notifier = INotify::Notifier.new
				notifier.watch(@filename, :modify, :attrib) do |event|
					p "#{@filename} was created"
					p event.flags.to_s
					p event.to_s
					@beaconInfo.clear
					beaconFile = File.open(@filename, "r")
					beaconContent = beaconFile.read
					beaconFile.close
					p beaconContent
					beaconContent = beaconContent[0, beaconContent.size - 1]
					unless beaconContent.nil?
						CGI::parse(beaconContent).each do |key, val|
							@beaconInfo[key] = val
						end
					end
					p @beaconInfo
				end
				while true
					p "waiting..."
					if IO.select([notifier.to_io], nil, nil, nil)
						notifier.process				
					end
				end
			end
		end
		def join
			p "waiting for thread to join"
			@thread.join
		end
		def getInfo key = nil
		end
	end
end
