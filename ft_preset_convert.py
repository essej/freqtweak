#!/usr/bin/env python

# this script converts old (before 0.5.0) FreqTweak presets into
# the new format in place.  (the original preset files are left for backward
# compatibility

import sys,os,shutil,string



def write_channel_node(wfile, info, pos):
    # writes out entire channel node with
    # known fixed modules and order

    wfile.write('    <Channel pos="%s" input_gain="%s" mix_ratio="%s" bypassed="%s" muted="%s">\n' % \
                (pos, info['input_gain'], info['mix_ratio'], info['bypassed'], info['muted']))

    wfile.write('     <ProcMods>\n')

    wfile.write('      <ProcMod pos="0" name="EQ">\n')
    wfile.write('        <Filter pos="0" name="freq" linked="%s" bypassed="%s" file="0_0_freq.filter">\n' % \
                (info['freq_bypassed'], info['freq_linked']))
    wfile.write('         <Extra height="68" minimized="0" xscale="0" gridsnap="0" grid="2" gridlines="0"/>\n')
    wfile.write('        </Filter>\n')
    wfile.write('      </ProcMod>\n')

    wfile.write('      <ProcMod pos="1" name="Pitch">\n')
    wfile.write('       <Filter pos="0" name="scale" linked="%s" bypassed="%s" file="0_1_scale.filter">\n' % \
                (info['scale_bypassed'], info['scale_linked']))
    wfile.write('         <Extra height="68" minimized="0" xscale="0" gridsnap="0" grid="1" gridlines="0"/>\n')
    wfile.write('       </Filter>\n')
    wfile.write('      </ProcMod>\n')

    wfile.write('      <ProcMod pos="2" name="Gate">\n')
    wfile.write('        <Filter pos="0" name="inverse_gate" linked="%s" bypassed="%s" file="0_2_inverse_gate.filter">\n' % \
                (info['inverse_gate_bypassed'], info['inverse_gate_linked']))
    wfile.write('          <Extra height="68" minimized="0" xscale="0" gridsnap="0" grid="2" gridlines="0"/>\n')
    wfile.write('        </Filter>\n')
    wfile.write('        <Filter pos="1" name="gate" linked="%s" bypassed="%s" file="0_2_gate.filter">\n' % \
                (info['gate_bypassed'], info['gate_linked']))
    wfile.write('          <Extra/>\n')
    wfile.write('        </Filter>\n')
    wfile.write('      </ProcMod>\n')

    wfile.write('      <ProcMod pos="3" name="Delay">\n')
    wfile.write('        <Filter pos="0" name="delay" linked="%s" bypassed="%s" file="0_3_delay.filter">\n' % \
                (info['delay_bypassed'], info['delay_linked']))
    wfile.write('         <Extra height="68" minimized="0" xscale="0" gridsnap="0" grid="2" gridlines="0"/>\n')
    wfile.write('        </Filter>\n')
    wfile.write('        <Filter pos="1" name="feedback" linked="%s" bypassed="%s" file="0_3_feedback.filter">\n' % \
                (info['feedback_bypassed'], info['feedback_linked']))
    wfile.write('          <Extra height="68" minimized="0" xscale="0" gridsnap="0" grid="3" gridlines="0"/>\n')
    wfile.write('        </Filter>\n')
    wfile.write('      </ProcMod>\n')

    wfile.write('     </ProcMods>\n')


    wfile.write('     <Inputs>\n')
    for port in string.split(info['input_ports'], ','):
        wfile.write('       <Port name="%s"/>\n' % port)
    wfile.write('     </Inputs>\n')

    wfile.write('     <Outputs>\n')
    for port in string.split(info['output_ports'], ','):
        wfile.write('       <Port name="%s"/>\n' % port)
    wfile.write('     </Outputs>\n')


    wfile.write('    </Channel>\n')

# the main script follows

preset_dir = os.path.join(os.getenv('HOME', '.'), '.freqtweak', 'presets')

if len(sys.argv) > 1:
    preset_dir = sys.argv[1]


try:
    os.chdir(preset_dir)

except OSError, ex:
    print 'Error accessing preset directory %s' % preset_dir
    sys.exit(2)


# each directory in the presets dir is a preset
preset_names = os.listdir(preset_dir)


for pname in preset_names:
    dirname = os.path.join(preset_dir, pname) 
    if not os.path.isdir(dirname):
        continue

    os.chdir(dirname)

    if os.path.exists('config.xml'):
        print 'Already converted: %s' % pname
        continue
    elif not os.path.exists('config.0'):
        print 'No old preset in %s' % dirname
        continue
    
    orig_info = []
    wfile = None
    try:
        wfile = open('config.xml', 'w+')
    except IOError:
        print 'error opening new %s/config.xml' % pname
        continue

    # write xml initial stuff
    wfile.write('<?xml version="1.0"?>\n')
    wfile.write('<Preset version="0.5.0">\n')


    for chan in range(4):
        if not os.path.exists('config.%d' % chan):
            break

        # copy filter files to new names
        shutil.copy('freq.%d.filter' % chan, '%d_0_freq.filter' % chan)
        shutil.copy('scale.%d.filter' % chan, '%d_1_scale.filter' % chan)
        shutil.copy('gate.%d.filter' % chan, '%d_2_gate.filter' % chan)
        shutil.copy('inverse_gate.%d.filter' % chan, '%d_2_inverse_gate.filter' % chan)
        shutil.copy('delay.%d.filter' % chan, '%d_3_delay.filter' % chan)
        shutil.copy('feedback.%d.filter' % chan, '%d_3_feedback.filter' % chan)

        # now parse config
        info = {}
        cfile = None
        
        try:
            cfile = open('config.%d' % chan, 'r')
        except IOError,ex:
            print "error opening config.%d" % chan
            continue

        lines = cfile.readlines()
        for line in lines:
            line = string.strip(line)
            if not line or line[0] == '#':
                continue

            try:
                key,val = string.split(line, '=', 1)
            except:
                continue

            info[key] = val

        if chan==0:
            # write params
            wfile.write('  <Params fft_size="%s" windowing="%s" update_speed="%s" oversamp="%s" tempo="%s" max_delay="%s" />\n' % \
                        (info['fft_size'], info['windowing'], info['update_speed'], info['oversamp'], info['tempo'], info['max_delay']))
            wfile.write('  <Channels>\n')

        # write Channel node
        write_channel_node(wfile, info, chan)


    wfile.write('  </Channels>\n')
    wfile.write('</Preset>\n')

    wfile.close()
    print 'Converted: %s' % pname
