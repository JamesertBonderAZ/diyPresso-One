pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                checkout scm
                sh 'platformio run'
            }
        }

        stage('Flash via Raspberry Pi') {
            steps {
                sh '''
                    scp .pio/build/mkr_wifi1010/firmware.bin qteal@gitlabrunner.local:/tmp/firmware.bin
                    
                    ssh qteal@gitlabrunner.local << 'EOF'
                        DEVICE=$(readlink -f /dev/dut || echo "/dev/ttyACM0")
                        arduino-cli upload -b arduino:samd:mkrwifi1010 -p "$DEVICE" --input-file /tmp/firmware.bin
                        rm -f /tmp/firmware.bin
                    EOF
                '''
            }
        }

        stage('Archive') {
            steps {
                archiveArtifacts '.pio/build/**/firmware.*'
            }
        }
    }
}
