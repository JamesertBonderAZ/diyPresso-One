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

        stage('Build Firmware') {
            agent {
                docker {
                    image 'ghcr.io/pbrier/platformio:latest'
                    args '-e HOME=$PWD -v $PWD:$PWD -w $PWD'
                }
            }
            steps {
                sh 'platformio run'
            }
        }

        stage('Archive Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/mkr_wifi1010/firmware.bin', fingerprint: true
            }
        }

    }
}
