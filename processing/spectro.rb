#! /usr/bin/env ruby

require 'time'

require 'pp'

class Ensemble

    attr_accessor :dut
    attr_accessor :measurements

    def initialize
        self.measurements = {}
    end

end

class Measurement

    attr_accessor :sensor
    attr_accessor :dut
    attr_accessor :time
    attr_accessor :X, :Y, :Z
    attr_accessor :x, :y
    attr_accessor :R, :G, :B
    attr_accessor :spectrum

end

def read_csv(filename)

    m =  Measurement.new

    File.readlines(filename).each_with_index do |line, line_nr|
        fields = line.split(/\s*,\s*/)
        puts fields

        m.sensor = fields[1] if fields[0] == "Model Name"
        m.time = Time.parse(fields[1].gsub(/_/, ' ')) if fields[0] == "Time"
        m.dut = fields[1] if fields[0] == "DUT"
        m.X = fields[1].to_f if fields[0] == "X"
        m.Y = fields[1].to_f if fields[0] == "Y"
        m.Z = fields[1].to_f if fields[0] == "Z"
        m.x = fields[1].to_f if fields[0] == "x"
        m.y = fields[1].to_f if fields[0] == "y"

        if fields[0] =~ /(\d+)nm/
            m.spectrum = {} unless m.spectrum
            m.spectrum[$1.to_i] = fields[1].to_f
        end

    end

    pp m
    m
end

def read_ensemble(filenames)

    e = Ensemble.new
    filenames.each do |f|
        m = read_csv(f)
        e.measurements[m.sensor] = m
    end

    e
end

e = read_ensemble(ARGV)
pp e
