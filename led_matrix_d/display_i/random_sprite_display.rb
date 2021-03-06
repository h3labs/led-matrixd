module LedMatrixD
	module DisplayI
		class RandomSpriteDisplay < LedMatrixD::Display
			def initialize(iniConfig)
				@spriteDirname = iniConfig['FILE SYSTEM']['image_basedir']+iniConfig['FILE SYSTEM']['sprite_dir']
				@spriteDuration = iniConfig['TIMING']['sprite_dur'].to_f / 1000
				@spriteIterations = iniConfig['ITERATIONS']['random_times']
				@spriteArray = Dir["#{@spriteDirname + '*'}.ppm"]
				@spriteHash = {}
				x = FFI::MemoryPointer.new :int
				y = FFI::MemoryPointer.new :int
				@spriteArray.each do |filename|
					image = FFI::MemoryPointer.new :pointer
					LedMatrixD::Native.load_image filename, image, x, y
					xVal = 0
					yVal = 0
					xVal = x.read_int
					yVal = y.read_int
					@spriteHash[filename] = {:content => image, :x => x, :y => y}
				end
				x.free
				y.free
			end
			def show()
				@spriteIterations.times do
					image_name = @spriteArray.sample
					image = @spriteHash[image_name][:content]
					$logger.info "RandomSpriteDisplay\n" +
						"\t\twith file #{image_name}\n" +
						"\t\tfor #{@spriteDuration}s"
					unless image.nil?
						LedMatrixD::Native.draw_image image, 0, 0
					else
						$logger.error "tried to draw #{image_name}"
						$logger.error "image is nil"
					end
					sleep(@spriteDuration)
				end	
			end
			def clean
				@spriteHash.each do |k, v|
					LedMatrixD::Native.free_image v[:content]
				end
			end
		end
	end
end
