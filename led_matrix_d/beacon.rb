
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
				basename = File.basename(@filename)
				dirWatcher = notifier.watch(File.dirname(@filename), :create, :attrib, :modify) do |event|
					if basename == event.name
						p event.flags.to_s
						p event.name
						@beaconInfo.clear
						beaconFile = File.open(@filename, "r")
						beaconContent = beaconFile.read
						beaconFile.close
						p beaconContent
						unless beaconContent.nil?
							if beaconContent[-1,1] == "\n"
								beaconContent = beaconContent[0, beaconContent.size - 1]
							end
							CGI::parse(beaconContent).each do |key, val|
								@beaconInfo[key] = val
							end
						end
						p @beaconInfo
					end
				end
				while true
					if IO.select([notifier.to_io], nil, nil, nil)
						notifier.process				
					end
				end
			end
		end
		def join
			p "waiting for thread to join"
			while true
				sleep(5)
				p @beaconInfo
			end
			@thread.join
		end
		def getInfo key = nil
			return @beaconInfo[key]
		end
	end
end
