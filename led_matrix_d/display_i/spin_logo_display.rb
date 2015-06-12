module LedMatrixD
	module DisplayI
		class SpinLogoDisplay
			def intialize(iniConfig)
				@frameDuration = iniConfig['TIMING']['atom_frame_dur']
				@iterations = iniConfig['TIMING']['atom_spin']
				@directory = iniConfig['FILE SYSTEM']['atom_dir']
				@frameNumbers = []
				@frameInfo = {}
				@frameFiles = Dir[imageBaseDir + @directory + '*.ppm']
				@frameFiles.each do |filename|
					p filename	
					number = filename[/([0-9]{3})\.ppm$/, 1]
					p "loading frame[#{number}] with file #{filename}"
					@frameNumbers.push(number)
					@frameInfo[number] = {
						:image => nil,
						:w => 0,
						:h => 0,
						:filename => filename
					}
				end	
			end
			def show
				wait(2)	
			end	
			def clean
			end
		end
	end
end
