pipeline {
    agent any

    stages {
        stage('Hello') {
            steps {
                echo 'Hello Pipeline'
            }
        }

        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Install PlatformIO') {
            steps {
                bat '''
                    pip install --upgrade pip
                    pip install --upgrade platformio
                '''
            }
        }

        stage('Build Firmware') {
            steps {
                bat 'platformio run'
            }
        }

        stage('Archive Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/mkr_wifi1010/firmware.bin'
            }
        }
    }
}
