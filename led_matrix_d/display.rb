module LedMatrixD
	class Display
		def show
			raise '#{self.class.to_s} has not defined the show method'
		end	
	end
end
