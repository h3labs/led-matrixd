require 'rubygems'
require 'sinatra'
require 'warden'
require 'bundler'
require 'tilt/erb'
require 'byebug'
Bundler.require
require_relative 'User.rb'
require_relative 'BeaconReader.rb'

Warden::Strategies.add(:password) do
  def valid?
    params['user'] && params['user']['username'] && params['user']['password']
  end

  def authenticate!
    user = User.first(username: params['user']['username'])

    if user.nil?
      throw(:warden, message: "The username you entered does not exist.")
    elsif user.authenticate(params['user']['password'])
      success!(user)
    else
      throw(:warden, message: "The username and password combination ")
    end
  end
end

class NeonWebServer < Sinatra::Application
  set :sessions, true
  register Sinatra::Flash
  set :session_secret, "supersecret"
  set :port, 8080

  use Warden::Manager do |config|
    # Tell Warden how to save our User info into a session.
    # Sessions can only take strings, not Ruby code, we'll store
    # the User's `id`
    config.serialize_into_session{|user| user.id }
    # Now tell Warden how to take what we've stored in the session
    # and get a User from that information.
    config.serialize_from_session{|id| User.get(id) }

    config.scope_defaults :default,
      # "strategies" is an array of named methods with which to
      # attempt authentication. We have to define this later.
      strategies: [:password],
      # The action is a route to send the user to when
      # warden.authenticate! returns a false answer. We'll show
      # this route below.
      action: 'auth/unauthenticated'
    # When a user tries to log in and cannot, this specifies the
    # app to send the user to.
    config.failure_app = self
  end

  def render_with_warden(page, hash = {})
    hash.merge!({:warden => env['warden'], :beacon => BeaconReader.new})
    erb page, :layout => :app, :locals => hash 
  end

  def only_authorized
    if env['warden'].authenticate 
      yield
    else
      flash[:error] = 'Restricted page. You must login.'
      redirect '/'
    end
  end

  def if_user_has_id(id)
    only_authorized do
      id = id.to_i
      if id == env['warden'].user.id
        yield
      else
        flash[:error] = 'You can\'t change others users information'
        if session[:return_to].nil?
          redirect '/'
        else
          redirect session[:return_to]
        end
      end
    end
  end

  get '/' do
    render_with_warden(:index)
  end

  get '/auth/login' do
    render_with_warden(:login)
  end

  post '/auth/login' do
    env['warden'].authenticate!

    flash[:success] = env['warden'].message

    if session[:return_to].nil?
      redirect '/'
    else
      redirect session[:return_to]
    end
  end

  get '/auth/logout' do
    if env['warden'].authenticated?
      env['warden'].raw_session.inspect
      env['warden'].logout
      flash[:success] = 'Successfully logged out'
    end
    redirect '/'
  end

  post '/auth/unauthenticated' do
    session[:return_to] = env['warden.options'][:attempted_path] if session[:return_to].nil?

    # Set the error and use a fallback if the message is not defined
    flash[:error] = env['warden.options'][:message] || "You must log in"
    redirect '/auth/login'
  end

  get '/beacon_change' do
    only_authorized do
      render_with_warden(:beacon_change)
    end
  end

  post '/beacon_change' do
    only_authorized do
      beacon = BeaconReader.new
      errors = beacon.write_hash(params[:beacon_info], env['warden'].user)
      if errors.size == 0
        flash[:success] = "Successfully changed the Beacon file" 
      else
        flash[:error] = errors.to_s
      end
      render_with_warden(:beacon_change)
    end
  end

  get '/user/:id' do
    if_user_has_id params[:id] do
      render_with_warden(:user, {:user => env['warden'].user})
    end
  end
  
  get '/*' do
    flash[:error] = 'Restricted page. You must login.'
    redirect '/auth/login'
  end
end
