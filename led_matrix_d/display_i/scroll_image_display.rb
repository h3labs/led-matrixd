module LedMatrixD
	module DisplayI
		class ScrollImageDisplay < LedMatrixD::Display
			def initialize(iniConfig, which)
				super iniConfig
				@scrollDur = iniConfig['TIMING']['scroll_ms']
				@iterations = iniConfig['ITERATIONS'][which + '_scroll']
				files = Dir[@imageBaseDir + which + '.ppm']
				@filename = files[0] unless files.size == 0
				@image = FFI::MemoryPointer.new :pointer
				w = FFI::MemoryPointer.new :int	
				h = FFI::MemoryPointer.new :int	
				$logger.info "loading file [#{@filename}] for scrolling"
				LedMatrixD::Native.load_image @filename, @image, w, h unless @filename.nil?
				@w = w.read_int
				@h = h.read_int
				w.free
				h.free
			end
			def show
				$logger.info "ScrollImageDisplay\n" +
					"\t\twith file #{@filename}\n" +
					"\t\tfor #{@iterations}\n" +
					"\t\tfor #{@scrollDur}s"
				unless @filename.nil?
					@iterations.times do
						LedMatrixD::Native.scroll_image @image, @scrollDur
					end
				end		
			end
			def clean
				LedMatrixD::Native.free_image @image unless @filename.nil?
			end
		end
	end
end
