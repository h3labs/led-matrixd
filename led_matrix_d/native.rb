
module LedMatrixD
	module Native
		extend FFI::Library
		ffi_lib_flags :local
		ffi_lib	'led-matrixd.so'
		attach_function :init, [], :void
		attach_function :clear, [], :void
		attach_function :load_image, [:string, :pointer, :pointer, :pointer], :void
		attach_function :free_image, [:pointer], :void
		attach_function :draw_image, [:pointer, :int, :int], :void
		attach_function :scroll_image, [:pointer, :int], :void 
		attach_function :run_demo, [:int, :int, :int], :void
		#attach_function :draw_text, [:string], :void
		#attach_function :scroll_text, [:string], :void
	end
end
