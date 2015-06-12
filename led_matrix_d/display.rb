module LedMatrixD
	class Display
		def show
			raise '#{self.class.to_s} has not defined the show method'
		end	
		def clean
			p '#{self.class.to_s} does not clean'
		end
	end
end
