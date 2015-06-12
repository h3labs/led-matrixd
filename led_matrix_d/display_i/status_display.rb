module LedMatrixD
	module DisplayI
		class StatusDisplay < LedMatrixD::Display
			def initialize(iniConfig, beacon)
				super iniConfig
				@scrollDur = iniConfig['TIMING']['scroll_ms']
				@iterations = iniConfig['ITERATIONS']['status_scroll']
				@statusArray = ['open', 'closed']
				@statusInfo = {}
				@beacon = beacon
				w = FFI::MemoryPointer.new :int	
				h = FFI::MemoryPointer.new :int	
				@statusArray.each do |status|
					files = Dir[@imageBaseDir + status +'.ppm']
					filename = files[0] unless files.size == 0
					image = FFI::MemoryPointer.new :pointer
					p "loading status file [#{filename}] for scrolling"
					LedMatrixD::Native.load_image filename, image, w, h unless filename.nil?
					@statusInfo[status] = {
						:filename => filename,
						:image => image,
						:w => w.read_int,
						:h => h.read_int
					}
				end
				w.free
				h.free
			end
			def show
				currentStatus = @beacon.getInfo 'status'
				currentStatus = 'closed' if currentStatus.nil?
				unless currentStatus.nil? 
					statusI = @statusInfo[currentStatus]
					p "current status #{currentStatus}"
					unless statusI[:filename].nil?
						@iterations.times do
							LedMatrixD::Native.scroll_image statusI[:image], @scrollDur
						end
					end		
				end
			end
			def clean
				@statusInfo.each do |k,v|
					p "freeing status[#{k}] image [#{v[:filename]}"
					LedMatrixD::Native.free_image v[:image] unless v[:filename].nil?
				end
			end
		end
	end
end
