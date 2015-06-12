module LedMatrixD
	class Display
		def initialize(iniConfig)
			@imageBaseDir = iniConfig['FILE SYSTEM']['image_basedir']
		end
		def show
			raise '#{self.class.to_s} has not defined the show method'
		end	
		def clean
			p '#{self.class.to_s} does not clean'
		end
		def imageBaseDir
			@imageBaseDir
		end
	end
end
