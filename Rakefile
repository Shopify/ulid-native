# frozen_string_literal: true

require('rake/extensiontask')
require('bundler/gem_tasks')

gemspec = Gem::Specification.load('ulid-native.gemspec')
Rake::ExtensionTask.new do |ext|
  ext.name = 'ulid'
  ext.ext_dir = 'ext/ulid'
  ext.lib_dir = 'lib/ulid'
  ext.gem_spec = gemspec
end

require('rake/testtask')

Rake::TestTask.new(:test) do |t|
  t.libs << 'test'
  t.libs << 'lib'
  t.test_files = FileList['test/**/*_test.rb']
end

task(default: [:compile, :test])
