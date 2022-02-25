# frozen_string_literal: true

lib = File.expand_path('lib', __dir__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require('ulid/version')

Gem::Specification.new do |spec|
  spec.name          = 'ulid-native'
  spec.version       = ULID::VERSION
  spec.authors       = ['Burke Libbey']
  spec.email         = ['burke.libbey@shopify.com']

  spec.license       = 'MIT'

  spec.summary       = 'Native ULID implementation'
  spec.description   = spec.summary
  spec.homepage      = 'https://github.com/Shopify/ulid-native'

  spec.metadata = {
    'bug_tracker_uri' => 'https://github.com/Shopify/ulid-native/issues',
    'source_code_uri' => 'https://github.com/Shopify/ulid-native',
    'allowed_push_host' => 'https://rubygems.org',
  }

  spec.files = %x(git ls-files -z ext lib).split("\x0") + ['LICENSE.txt', 'README.md']
  spec.require_paths = ['lib']

  spec.required_ruby_version = '>= 2.6.0'

  spec.platform   = Gem::Platform::RUBY
  spec.extensions = ['ext/ulid/extconf.rb']
end
