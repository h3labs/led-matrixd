module LedMatrixD
	module DisplayI
		class DemoDisplay < LedMatrixD::Display
			def initialize(iniConfig, which=nil)
				@duration = iniConfig['TIMING']['demo_dur']
				@demoArray = 0..7.to_a
				@which = which
			end
			def show
				$logger.info "DemoDisplay " +
					"running demo number #{@which}" +
					"for #{@duration}ms"
				unless @which.nil?	
					LedMatrixD::Native.run_demo @which, @duration, 400
				else
					LedMatrixD::Native.run_demo @demoArray.sample, @duration, 400
				end
			end
		end 
	end
end
