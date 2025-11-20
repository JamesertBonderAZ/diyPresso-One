pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Install Python & PlatformIO') {
            steps {
                sh '''
                    # Install python + pip if missing (jenkins user has sudo rights by default)
                    which python3 || (apt-get update && apt-get install -y python3 python3-pip)

                    # Install/upgrade PlatformIO
                    python3 -m pip install --upgrade --user platformio

                    # Add pio to PATH for this build
                    export PATH="$HOME/.local/bin:$PATH"
                    platformio --version
                '''
            }
        }

        stage('Build Firmware') {
            steps {
                sh '''
                    export PATH="$HOME/.local/bin:$PATH"
                    platformio run
                '''
            }
        }

        stage('Archive Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/**/firmware.*',
                                 fingerprint: true,
                                 onlyIfSuccessful: true
            }
        }
    }

    post {
        always {
            cleanWs()
        }
    }
}
