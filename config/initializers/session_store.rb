# Be sure to restart your server when you modify this file.

# Your secret key for verifying cookie session data integrity.
# If you change this key, all old sessions will become invalid!
# Make sure the secret is at least 30 characters and all random, 
# no regular words or you'll be exposed to dictionary attacks.
ActionController::Base.session = {
  :key         => '_waitlist_session',
  :secret      => '213c425604fd0fd5c1db6adce8efff2f5b79279233b1a9d201b14459c5121da4dac5515e833ef5fc939d92173abf6a0e72139ff19e71b6c57bb2dde2bac6d939'
}

# Use the database for sessions instead of the cookie-based default,
# which shouldn't be used to store highly confidential information
# (create the session table with "rake db:sessions:create")
# ActionController::Base.session_store = :active_record_store
