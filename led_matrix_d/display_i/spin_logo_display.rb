module LedMatrixD
	module DisplayI
		class SpinLogoDisplay < LedMatrixD::Display
			def initialize(iniConfig)
				super iniConfig
				@frameDuration = iniConfig['TIMING']['atom_frame_dur'].to_f / 1000
				@iterations = iniConfig['ITERATIONS']['atom_spin']
				@directory = iniConfig['FILE SYSTEM']['atom_dir']
				@frameNumbers = []
				@frameInfo = {}
				@frameFiles = Dir[@imageBaseDir + @directory + '*.ppm']
				w = FFI::MemoryPointer.new :int
				h = FFI::MemoryPointer.new :int
				@frameFiles.each do |filename|
					number = filename[/([0-9]{3})\.ppm$/, 1].to_i
					logger.info "loading frame[#{number}] with file #{filename}"
					@frameNumbers.push(number)
					image = FFI::MemoryPointer.new :pointer
					LedMatrixD::Native.load_image filename, image, w, h
					@frameInfo[number] = {
						:image => image,
						:w => w.read_int,
						:h => h.read_int,
						:filename => filename
					}
				end	
				w.free
				h.free
				@frameNumbers.sort!
				p "frames -> #{@frameNumbers.to_s}"
			end
			def show
				logger.info "SpinLogoDisplay \n" + 
					"running #{@iterations} iterations\n" +
					"for #{@frameNumbers.size}\n" +
					"each for #{@frameDuration}s"
				@iterations.times do 
					@frameNumbers.each do |frame|
						frameInfo = @frameInfo[frame]
						LedMatrixD::Native.draw_image frameInfo[:image], 0, 0
						sleep(@frameDuration)
					end
				end
			end	
			def clean
				@frameInfo.each do |k,v|
					logger.info "freeing frame[#{k}] with file #{v[:filename]}"
					LedMatrixD::Native.free_image v[:image] unless v[:image].nil?
					v[:image].free unless v[:image].nil?
				end
			end
		end
	end
end
