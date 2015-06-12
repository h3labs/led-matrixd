module LedMatrixD
	class Display
		def initialize(iniConfig)
			@imageBaseDir = iniConfig['FILE SYSTEM']['image_basedir']
		end
		def show
			$logger.error '#{self.class.to_s} has not defined the show method'
			raise '#{self.class.to_s} has not defined the show method'
		end	
		def clean
			$logger.warn '#{self.class.to_s} does not clean'
		end
		def imageBaseDir
			@imageBaseDir
		end
	end
end
