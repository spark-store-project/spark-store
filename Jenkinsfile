pipeline {
  agent any
  stages {
    stage('build') {
      agent {
        docker {
          image 'shenmo7192/uos-21-dtk5.4:1.0'
        }
      }
      steps {
        sh 'dpkg-buildpackage && tree .'
      }
    }

    stage('archive') {
      steps {
        archiveArtifacts(artifacts: 'build/src/spark-store', allowEmptyArchive: true, defaultExcludes: true)
      }
    }

    stage('send') {
      agent {
        dockerfile {
          filename '.gitee/Dockerfile'
        }
      }
      environment {
        gitee_token = credentials('1')
      }
      steps {
        sh 'python3 .gitee/callback.py'
      }
    }
  }
}
