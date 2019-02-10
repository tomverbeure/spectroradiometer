#! /usr/bin/env ruby

require 'time'
require 'json'

require 'pp'

class Spectrum

    attr_accessor :buckets, :bucket_width

    def initialize
        self.buckets = {}
    end

    CMF_XYZ = {
        380 => [ 0.0014, 0.0   , 0.0065 ],
        385 => [ 0.0022, 0.0001, 0.0105 ],
        390 => [ 0.0042, 0.0001, 0.0201 ],
        395 => [ 0.0076, 0.0002, 0.0362 ],
        400 => [ 0.0143, 0.0004, 0.0679 ],
        405 => [ 0.0232, 0.0006, 0.1102 ],
        410 => [ 0.0435, 0.0012, 0.2074 ],
        415 => [ 0.0776, 0.0022, 0.3713 ],
        420 => [ 0.1344, 0.004 , 0.6456 ],
        425 => [ 0.2148, 0.0073, 1.0391 ],
        430 => [ 0.2839, 0.0116, 1.3856 ],
        435 => [ 0.3285, 0.0168, 1.623  ],
        440 => [ 0.3483, 0.023 , 1.7471 ],
        445 => [ 0.3481, 0.0298, 1.7826 ],
        450 => [ 0.3362, 0.038 , 1.7721 ],
        455 => [ 0.3187, 0.048 , 1.7441 ],
        460 => [ 0.2908, 0.06  , 1.6692 ],
        465 => [ 0.2511, 0.0739, 1.5281 ],
        470 => [ 0.1954, 0.091 , 1.2876 ],
        475 => [ 0.1421, 0.1126, 1.0419 ],
        480 => [ 0.0956, 0.139 , 0.813  ],
        485 => [ 0.0580, 0.1693, 0.6162 ],
        490 => [ 0.0320, 0.208 , 0.4652 ],
        495 => [ 0.0147, 0.2586, 0.3533 ],
        500 => [ 0.0049, 0.323 , 0.272  ],
        505 => [ 0.0024, 0.4073, 0.2123 ],
        510 => [ 0.0093, 0.503 , 0.1582 ],
        515 => [ 0.0291, 0.6082, 0.1117 ],
        520 => [ 0.0633, 0.71  , 0.0782 ],
        525 => [ 0.1096, 0.7932, 0.0573 ],
        530 => [ 0.1655, 0.862 , 0.0422 ],
        535 => [ 0.2257, 0.9149, 0.0298 ],
        540 => [ 0.2904, 0.954 , 0.0203 ],
        545 => [ 0.3597, 0.9803, 0.0134 ],
        550 => [ 0.4334, 0.995 , 0.0087 ],
        555 => [ 0.5121, 1.0000, 0.0057 ],
        560 => [ 0.5945, 0.995 , 0.0039 ],
        565 => [ 0.6784, 0.9786, 0.0027 ],
        570 => [ 0.7621, 0.952 , 0.0021 ],
        575 => [ 0.8425, 0.9154, 0.0018 ],
        580 => [ 0.9163, 0.87  , 0.0017 ],
        585 => [ 0.9786, 0.8163, 0.0014 ],
        590 => [ 1.0263, 0.757 , 0.0011 ],
        595 => [ 1.0567, 0.6949, 0.001  ],
        600 => [ 1.0622, 0.631 , 0.0008 ],
        605 => [ 1.0456, 0.5668, 0.0006 ],
        610 => [ 1.0026, 0.503 , 0.0003 ],
        615 => [ 0.9384, 0.4412, 0.0002 ],
        620 => [ 0.8544, 0.381 , 0.0002 ],
        625 => [ 0.7514, 0.321 , 0.0001 ],
        630 => [ 0.6424, 0.265 , 0.0 ],
        635 => [ 0.5419, 0.217 , 0.0 ],
        640 => [ 0.4479, 0.175 , 0.0 ],
        645 => [ 0.3608, 0.1382, 0.0 ],
        650 => [ 0.2835, 0.107 , 0.0 ],
        655 => [ 0.2187, 0.0816, 0.0 ],
        660 => [ 0.1649, 0.061 , 0.0 ],
        665 => [ 0.1212, 0.0446, 0.0 ],
        670 => [ 0.0874, 0.032 , 0.0 ],
        675 => [ 0.0636, 0.0232, 0.0 ],
        680 => [ 0.0468, 0.017 , 0.0 ],
        685 => [ 0.0329, 0.0119, 0.0 ],
        690 => [ 0.0227, 0.0082, 0.0 ],
        695 => [ 0.0158, 0.0057, 0.0 ],
        700 => [ 0.0114, 0.0041, 0.0 ],
        705 => [ 0.0081, 0.0029, 0.0 ],
        710 => [ 0.0058, 0.0021, 0.0 ],
        715 => [ 0.0041, 0.0015, 0.0 ],
        720 => [ 0.0029, 0.001 , 0.0 ],
        725 => [ 0.0020, 0.0007, 0.0 ],
        730 => [ 0.0014, 0.0005, 0.0 ],
        735 => [ 0.0010, 0.0004, 0.0 ],
        740 => [ 0.0007, 0.0002, 0.0 ],
        745 => [ 0.0005, 0.0002, 0.0 ],
        750 => [ 0.0003, 0.0001, 0.0 ],
        755 => [ 0.0002, 0.0001, 0.0 ],
        760 => [ 0.0002, 0.0001, 0.0 ],
        765 => [ 0.0001, 0.0   , 0.0 ],
        770 => [ 0.0001, 0.0   , 0.0 ],
        775 => [ 0.0001, 0.0   , 0.0 ],
        780 => [ 0.0000, 0.0   , 0.0 ],
    }

    AS_BUCKETS = [ 410, 435, 460, 485, 510, 535, 560, 585, 610, 645, 680, 705, 730, 760, 810, 860, 900, 940]

    def self.x_cfm
        x = Spectrum.new
        x.bucket_width = 5

        CMF_XYZ.each { |nm, val| x.buckets[nm] = val[0] }

        x
    end

    def self.y_cfm
        y = Spectrum.new
        y.bucket_width = 5

        CMF_XYZ.each { |nm, val| y.buckets[nm] = val[1] }

        y
    end

    def self.z_cfm
        z = Spectrum.new
        z.bucket_width = 5

        CMF_XYZ.each { |nm, val| z.buckets[nm] = val[2] }

        z
    end

    def self.as_mask(target)
        s = Spectrum.new
        s.bucket_width = 20

        target.buckets.keys.each do |nm|
            s.buckets[nm] = 0.0

            AS_BUCKETS.each do |as_nm|
                val = Math.exp(-4.0 * Math.log(2) * ((as_nm-nm)**2.0) / (s.bucket_width**2.0) )
                s.buckets[nm] += val
            end
        end

        s
    end

    def resample(target)
        s_new = Spectrum.new

        target.buckets.each { |nm, val| s_new.buckets[nm] = 0.0 }
        s_new.bucket_width = target.bucket_width

        self.buckets.each do |old_nm, old_val|
            closest_nm = 10000
            s_new.buckets.keys.each do |new_nm|
                if (new_nm-old_nm).abs < (closest_nm-old_nm).abs
                    closest_nm = new_nm
                end
            end

            # FIXME: currently assume that all original buckets fall into a new bucket
            s_new.buckets[closest_nm] += old_val
        end

        s_new
    end

    def multiply(target)
        target_resample = target.resample(self)

        s_new = Spectrum.new
        s_new.bucket_width = self.bucket_width

        self.buckets.each do |nm, b_val|
            t_val = target_resample.buckets[nm]
            s_new.buckets[nm] = b_val * t_val
        end

        s_new
    end

    def sum_reduce
        sum = 0.0

        self.buckets.each { |nm, val| sum += val }

        sum
    end

    def visible_spectrum
        s_new = Spectrum.new
        s_new.bucket_width = self.bucket_width

        self.buckets.each { |nm, val| s_new.buckets[nm] = val if (nm >= 380 && nm <= 780) }

        s_new
    end

    def calc_primaries_abs
        x = Spectrum.x_cfm
        y = Spectrum.y_cfm
        z = Spectrum.z_cfm

        s_resample = self.visible_spectrum.resample(x)

        s_x = s_resample.multiply(x)
        s_y = s_resample.multiply(y)
        s_z = s_resample.multiply(z)

        x = s_x.sum_reduce
        y = s_y.sum_reduce
        z = s_z.sum_reduce

        [x,y,z]
    end

    def calc_primaries_rel
        x,y,z = self.calc_primaries_abs
        denom = x+y+z

        [x/denom, y/denom]
    end

    def to_csv
        s = self.buckets.sort.collect { |nm, val| "#{nm},#{val}" }.join("\n")
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

class Color

    attr_accessor :dut
    attr_accessor :measurements

    def initialize
        self.measurements = {}
    end

end

class Screen
    attr_accessor :dut
    attr_accessor :colors

    def initialize
        self.colors = {}
    end
end

def read_csv(filename)

    m =  Measurement.new

    File.readlines(filename).each_with_index do |line, line_nr|
        fields = line.split(/\s*,\s*/).collect{ |f| f.chomp }

        m.sensor = fields[1] if fields[0] == "Model Name"
        m.time = Time.parse(fields[1].gsub(/_/, ' ')) if fields[0] == "Time"
        m.dut = fields[1] if fields[0] == "DUT"
        m.X = fields[1].to_f if fields[0] == "X"
        m.Y = fields[1].to_f if fields[0] == "Y"
        m.Z = fields[1].to_f if fields[0] == "Z"
        m.x = fields[1].to_f if fields[0] == "x"
        m.y = fields[1].to_f if fields[0] == "y"

        if fields[0] =~ /(\d+)nm/
            m.spectrum = Spectrum.new unless m.spectrum
            m.spectrum.buckets[$1.to_i] = fields[1].to_f
        end

    end

    m
end

def read_colors(filenames)

    e = Color.new
    filenames.each do |f|
        m = read_csv(f)
        e.measurements[m.sensor] = m
    end

    e
end

def import_json(filename)
    screen = Screen.new

    basedir = File.dirname(filename)

    screen.colors["r"] = {}
    screen.colors["g"] = {}
    screen.colors["b"] = {}
    screen.colors["w"] = {}

    data_files = JSON.parse(File.open(filename).read)

    data_files.each do |sensor, colors|
        colors.each do |color, file|
            screen.colors[color][sensor] = read_csv(basedir + '/' + file)
        end
    end

    screen
end

if nil
    e = read_colors(ARGV)

    x = Spectrum.x_cfm
    y = Spectrum.y_cfm
    z = Spectrum.z_cfm

    gs_orig = e.measurements["GS1160"].spectrum
    gs_resample = e.measurements["GS1160"].spectrum.resample(x)
    gs_x = gs_resample.multiply(x)
    gs_y = gs_resample.multiply(y)
    gs_z = gs_resample.multiply(z)

    x = gs_x.sum_reduce
    y = gs_y.sum_reduce
    z = gs_z.sum_reduce

    gs_resample.buckets.each do |nm, val|
        puts "#{nm},#{val}"
    end

    puts
    puts x,y,z
    puts x/(x+y+z), y/(x+y+z)

end

def compare_spectra
end

if 1
    screen = import_json(ARGV[0])

    ["gs", "as"].each do |sensor|
        puts sensor
        [ 'r', 'g', 'b', 'w'].each do |c|
            x,y = screen.colors[c][sensor].spectrum.calc_primaries_rel
            puts "#{ c }: X %4.4f, Y %4.4f" % [x,y]
        end
    end

    s = Spectrum.as_mask(screen.colors['r']['gs'].spectrum)
    puts s.to_csv
end


