pipeline {
    agent any

    stages {
        stage('Build diyPresso-One') {
            steps {
                checkout scm
                sh 'platformio run'
            }
        }

        stage('Save Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/**/firmware.*', fingerprint: true
            }
        }
    }
}
