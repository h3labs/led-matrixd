module Web
	class BeaconReader
	  def initialize()
	    @filename = $iniConfig['FILE SYSTEM']['shop_status_flag']
	  end
	  def get_params
	    if File.exist?(@filename)
	      beaconFile = File.open(@filename, "r")
	      beaconContent = beaconFile.read
	      beaconFile.close
	      if beaconContent[-1,1] == "\n"
		beaconContent = beaconContent[0, beaconContent.size - 1]
	      end
	      unless beaconContent.nil?
		CGI::parse(beaconContent).each do |key, val|
		  yield key, val[0]
		end
	      end
	    end
	  end
	  def get_lastupdated
	    if File.exist?(@filename)
	      beaconFile = File.open(@filename, "r")
	      yield beaconFile.mtime
	      beaconFile.close
	    end
	  end
	  def get(keyed = 'status')
	    status = nil
	    get_params do |key, val|
	      if key.to_s == keyed
		status = val.to_s
		break
	      end
	    end
	    return status
	  end
	  def exists?
	    if File.exist?(@filename)
	      true
	    else
	      false
	    end
	  end
	  def write_hash(hash, user = nil)
	    errors = []
	    if user.nil?
	      errors.push 'Must be logged in as a User to write to beacon'
	      return errors
	    end
	    if not hash[:msg].is_a? String
	      errors.push 'Message is a not a string'
	    end
	    if not ['open','closed'].include? hash[:status]
	      errors.push 'Status can only be \'open\' or \'closed\''
	    end
	    if errors.size == 0
	      msg = CGI::escape(hash[:msg])  
	      status = CGI::escape(hash[:status])  
	      username = CGI::escape(user.username)
	      output = "msg=#{msg}&user=#{username}&status=#{status}"
	      File.open(@filename, "w") do |file|
		file.write output        
	      end
	    end
	    return errors
	  end
	end
end
