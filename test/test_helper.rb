# frozen_string_literal: true

$LOAD_PATH.unshift(File.expand_path("../lib", __dir__))

require("bundler/setup")
require("ulid")

require("tmpdir")
require("fileutils")

require("minitest/autorun")
require("mocha/minitest")
